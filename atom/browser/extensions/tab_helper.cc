// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/extensions/tab_helper.h"

#include <map>
#include <string>
#include <utility>
#include "atom/browser/extensions/atom_extension_api_frame_id_map_helper.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/web_contents.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/extension_messages.h"
#include "native_mate/dictionary.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(extensions::TabHelper);

namespace keys {
const char kIdKey[] = "id";
const char kTabIdKey[] = "tabId";
const char kActiveKey[] = "active";
const char kIncognitoKey[] = "incognito";
const char kWindowIdKey[] = "windowId";
const char kTitleKey[] = "title";
const char kUrlKey[] = "url";
const char kStatusKey[] = "status";
}

static std::map<int, std::pair<int, int>> render_view_map_;
static std::map<int, int> active_tab_map_;
static int32_t next_id = 1;

namespace extensions {

TabHelper::TabHelper(content::WebContents* contents)
    : content::WebContentsObserver(contents),
      script_executor_(
          new ScriptExecutor(contents, &script_execution_observers_)) {
  session_id_ = next_id++;
  RenderViewCreated(contents->GetRenderViewHost());
  contents->ForEachFrame(
      base::Bind(&TabHelper::SetTabId, base::Unretained(this)));
}

TabHelper::~TabHelper() {
}

void TabHelper::SetWindowId(const int32_t& id) {
  window_id_ = id;
  // Extension code in the renderer holds the ID of the window that hosts it.
  // Notify it that the window ID changed.
  web_contents()->SendToAllFrames(
      new ExtensionMsg_UpdateBrowserWindowId(MSG_ROUTING_NONE, window_id_));
}

void TabHelper::SetActive(bool active) {
  if (active)
    active_tab_map_[window_id_] = session_id_;
  else if (active_tab_map_[window_id_] == session_id_)
    active_tab_map_[window_id_] = -1;
}

void TabHelper::RenderViewCreated(content::RenderViewHost* render_view_host) {
  render_view_map_[session_id_] = std::make_pair(
      render_view_host->GetProcess()->GetID(),
      render_view_host->GetRoutingID());
}

void TabHelper::RenderFrameCreated(content::RenderFrameHost* host) {
  SetTabId(host);
}

void TabHelper::WebContentsDestroyed() {
  render_view_map_.erase(session_id_);
}

void TabHelper::SetTabId(content::RenderFrameHost* render_frame_host) {
  render_frame_host->Send(
      new ExtensionMsg_SetTabId(render_frame_host->GetRoutingID(),
                                session_id_));
}

void TabHelper::DidCloneToNewWebContents(
    content::WebContents* old_web_contents,
    content::WebContents* new_web_contents) {
  // When the WebContents that this is attached to is cloned,
  // give the new clone a TabHelper and copy state over.
  CreateForWebContents(new_web_contents);
}

bool TabHelper::ExecuteScriptInTab(
    const std::string extension_id,
    const std::string code_string,
    const mate::Dictionary& options) {
  extensions::ScriptExecutor* executor = script_executor();
  if (!executor)
    return false;

  bool all_frames = false;
  options.Get("allFrames", &all_frames);
  extensions::ScriptExecutor::FrameScope frame_scope =
      all_frames
          ? extensions::ScriptExecutor::INCLUDE_SUB_FRAMES
          : extensions::ScriptExecutor::SINGLE_FRAME;

  int frame_id = extensions::ExtensionApiFrameIdMap::kTopFrameId;
  options.Get("frameId", &frame_id);

  bool match_about_blank = false;
  options.Get("matchAboutBlank", &match_about_blank);

  extensions::UserScript::RunLocation run_at =
    extensions::UserScript::UNDEFINED;
  std::string run_at_string = "undefined";
  options.Get("runAt", &run_at_string);
  if (run_at_string == "document_start") {
    run_at = extensions::UserScript::DOCUMENT_START;
  } else if (run_at_string == "document_end") {
    run_at = extensions::UserScript::DOCUMENT_END;
  } else if (run_at_string == "document_idle") {
    run_at = extensions::UserScript::DOCUMENT_IDLE;
  }

  bool isolated_world = false;
  options.Get("isolatedWorld", &isolated_world);

  executor->ExecuteScript(
      HostID(HostID::EXTENSIONS, extension_id),
      extensions::ScriptExecutor::JAVASCRIPT,
      code_string,
      frame_scope,
      frame_id,
      match_about_blank ? extensions::ScriptExecutor::MATCH_ABOUT_BLANK
                        : extensions::ScriptExecutor::DONT_MATCH_ABOUT_BLANK,
      run_at,
      isolated_world ? extensions::ScriptExecutor::ISOLATED_WORLD
                     : extensions::ScriptExecutor::MAIN_WORLD,
      extensions::ScriptExecutor::DEFAULT_PROCESS,
      GURL(),  // No webview src.
      GURL(),  // No file url.
      false,  // user gesture
      extensions::ScriptExecutor::NO_RESULT,
      extensions::ScriptExecutor::ExecuteScriptCallback());
  return true;
}

// static
content::WebContents* TabHelper::GetTabById(int32_t tab_id,
                          content::BrowserContext* browser_context) {
  content::RenderViewHost* rvh =
      content::RenderViewHost::FromID(render_view_map_[tab_id].first,
                                      render_view_map_[tab_id].second);
  if (rvh) {
    auto contents = content::WebContents::FromRenderViewHost(rvh);
    if (extensions::ExtensionsBrowserClient::Get()->IsSameContext(
                                      browser_context,
                                      contents->GetBrowserContext())) {
      if (tab_id == extensions::TabHelper::IdForTab(contents))
        return contents;
    }
  }
  return NULL;
}

// static
base::DictionaryValue* TabHelper::CreateTabValue(
                                              content::WebContents* contents) {
  auto tab_id = IdForTab(contents);
  auto window_id = IdForWindowContainingTab(contents);

  base::DictionaryValue* result = new base::DictionaryValue();

  result->SetInteger(keys::kIdKey, tab_id);
  result->SetInteger(keys::kTabIdKey, tab_id);
  result->SetInteger(keys::kWindowIdKey, window_id);
  result->SetBoolean(keys::kIncognitoKey,
                     contents->GetBrowserContext()->IsOffTheRecord());
  result->SetBoolean(keys::kActiveKey, active_tab_map_[window_id] == tab_id);
  result->SetString(keys::kUrlKey, contents->GetURL().spec());
  result->SetString(keys::kTitleKey, contents->GetTitle());
  result->SetString(keys::kStatusKey, contents->IsLoading()
      ? "loading" : "complete");
  return result;
}

// static
int32_t TabHelper::IdForTab(const content::WebContents* tab) {
  const TabHelper* session_tab_helper =
      tab ? TabHelper::FromWebContents(tab) : NULL;
  return session_tab_helper ? session_tab_helper->session_id_ : -1;
}

// static
int32_t TabHelper::IdForWindowContainingTab(
    const content::WebContents* tab) {
  const TabHelper* session_tab_helper =
      tab ? TabHelper::FromWebContents(tab) : NULL;
  return session_tab_helper ? session_tab_helper->window_id_ : -1;
}

}  // namespace extensions