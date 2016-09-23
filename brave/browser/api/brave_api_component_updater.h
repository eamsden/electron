// Copyright 2016 Brave authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BRAVE_BROWSER_API_BRAVE_API_COMPONENT_UPDATER_H_
#define BRAVE_BROWSER_API_BRAVE_API_COMPONENT_UPDATER_H_

#include <string>

#include "atom/browser/api/trackable_object.h"
#include "base/callback.h"
#include "components/component_updater/component_updater_service.h"
#include "native_mate/handle.h"

// Just used to give access to OnDemandUpdater since it's private.
// Chromium has ComponentsUI which is a friend class, so we just
// do this hack here to gain access.
class ComponentsUI {
 public:
  void OnDemandUpdate(
      component_updater::ComponentUpdateService* cus,
      const std::string& component_id);
  bool GetComponentDetails(const std::string& id,
                           update_client::CrxUpdateItem* item) const;
};

using ReadyCallback = base::Callback<void(const base::FilePath&)>;

namespace brave {

namespace api {

class ComponentUpdater : public mate::TrackableObject<ComponentUpdater>,
                         public component_updater::ServiceObserver,
                         public ComponentsUI {
 public:
  static mate::Handle<ComponentUpdater> Create(v8::Isolate* isolate);

  // mate::TrackableObject:
  static void BuildPrototype(v8::Isolate* isolate,
                             v8::Local<v8::FunctionTemplate> prototype);

 protected:
  explicit ComponentUpdater(v8::Isolate* isolate);
  ~ComponentUpdater() override;
  void RegisterComponent(const std::string& extension_id);
  void CheckNow(const std::string& extension_id);
  void OnComponentRegistered(const std::string& extension_id);
  void OnComponentReady(
    const std::string& extension_id,
    const base::FilePath& install_dir);

  // Registers a component for auto updates and for the ability to be installed
  void RegisterComponentForUpdate(const std::string& public_key,
      const base::Closure& registered_callback,
      const ReadyCallback& ready_callback);

  // ServiceObserver implementation.
  void OnEvent(Events event, const std::string& id) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(ComponentUpdater);
};

}  // namespace api

}  // namespace brave

#endif  // BRAVE_BROWSER_API_BRAVE_API_COMPONENT_UPDATER_H_
