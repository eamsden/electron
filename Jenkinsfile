def startVM(name) {
  stage(name: "Create VM", concurrency: 1) {
    echo "Creating /Users/Shared/Jenkins/vagrant/electron-vagrant ${name}"
    withEnv(["VAGRANT_DOTFILE_PATH=.${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant up ${name}"
    }
  }
}

def stopVM(name) {
  stage("Stop VM") {
    echo "Stopping /Users/Shared/Jenkins/vagrant/electron-vagrant ${name}"
    withEnv(["VAGRANT_DOTFILE_PATH=.${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant halt ${name}"
    }
  }
}

def destroyVM(name) {
  stage("Destroy VM") {
    echo "Destroying /Users/Shared/Jenkins/vagrant/electron-vagrant ${name}"
    withEnv(["VAGRANT_DOTFILE_PATH=.${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant destroy -f ${name}"
    }
  }
}

def vmSSH(name, command) {
  withEnv(["VAGRANT_DOTFILE_PATH=.${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
    sh "vagrant ssh ${name} -c '${command}'"
  }
}

def setLinuxDisplay(name) {
  vmSSH(name, "export DISPLAY=:99.0")
}

def npmInstall(name, cmd = 'npm') {
  vmSSH(name, "cd electron && ${cmd} install npm@3.3.12")
}

def buildElectron() {
  stage(name: 'Build Electron', concurrency: 1) {
    stage('Clean') {
      deleteDir()
    }
    stage('Checkout') {
      checkout scm
    }
    stage('Bootstrap') {
      retry(3) {
        timeout(30) {
          sh "python script/clean.py"
          sh "npm install npm@3.3.12"
          sh "python script/bootstrap.py -v --target_arch ${env.TARGET_ARCH}"
        }
      }
    }
    stage('Lint') {
      sh "python script/cpplint.py"
    }
    stage('Build') {
      sh "python script/build.py -c R"
    }
    stage('Create Dist') {
      sh "python script/create-dist.py"
    }
    stage('Upload') {
      retry(3) {
        sh "python script/upload.py"
      }
    }
  }
}

def buildElectronVagrant(name, npmCmd = 'npm') {
  stage(name: "Build Electron VM ${name}", concurrency: 1) {
    stage('VM Clean') {
      deleteDir()
    }
    stage('VM Checkout') {
      retry(3) {
        vmSSH(name, "git clone https://github.com/brave/electron.git")
      }
    }
    stage('VM Bootstrap') {
      retry(3) {
        timeout(30) {
          vmSSH(name, "source ~/.profile && cd electron && python script/clean.py")
          npmInstall(name, npmCmd)
          vmSSH(name, "source ~/.profile && cd electron && python script/bootstrap.py -v --target_arch ${env.TARGET_ARCH}")
        }
      }
    }
    stage('VM Lint') {
      vmSSH(name, "source ~/.profile && cd electron && python script/cpplint.py")
    }
    stage('VM Build') {
      vmSSH(name, "source ~/.profile && cd electron && python script/build.py -c R")
    }
    stage('VM Create Dist') {
      vmSSH(name, "source ~/.profile && cd electron && python script/create-dist.py")
    }
    stage('VM Upload') {
      retry(3) {
        vmSSH(name, "source ~/.profile && cd electron && python script/upload.py")
      }
    }
  }
}

def installNode(name) {
  stage('install node') {
    retry(3) {
      vmSSH(name, "curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -")
    }
    vmSSH(name, "sudo apt-get install -y nodejs")
  }
}

def setEnvVagrant(name) {
  vmSSH(name, "echo \"export TARGET_ARCH=${env.TARGET_ARCH}\" >> ~/.profile")
  vmSSH(name, "echo \"export ELECTRON_S3_BUCKET=${env.ELECTRON_S3_BUCKET}\" >> ~/.profile")
  vmSSH(name, "echo \"export LIBCHROMIUMCONTENT_MIRROR=${env.LIBCHROMIUMCONTENT_MIRROR}\" >> ~/.profile")
  vmSSH(name, "echo \"export CI=${env.CI}\" >> ~/.profile")
  vmSSH(name, "echo \"export ELECTRON_RELEASE=${env.ELECTRON_RELEASE}\" >> ~/.profile")
  vmSSH(name, "echo \"export GYP_DEFINES=${env.GYP_DEFINES}\" >> ~/.profile")
  vmSSH(name, "echo \"export ELECTRON_S3_SECRET_KEY=${env.ELECTRON_S3_SECRET_KEY}\" >> ~/.profile")
  vmSSH(name, "echo \"export ELECTRON_S3_ACCESS_KEY=${env.ELECTRON_S3_ACCESS_KEY}\" >> ~/.profile")
  vmSSH(name, "echo \"export ELECTRON_GITHUB_TOKEN=${env.ELECTRON_GITHUB_TOKEN}\" >> ~/.profile")
  vmSSH(name, "source ~/.profile")
}

timestamps {
  withEnv([
    'ELECTRON_S3_BUCKET=brave-laptop-binaries',
    'LIBCHROMIUMCONTENT_MIRROR=https://s3.amazonaws.com/brave-laptop-binaries/libchromiumcontent',
    'CI=1',
    'ELECTRON_RELEASE=1'
]) {
    // LIBCHROMIUMCONTENT_COMMIT - get from previous job
    parallel (
//      mac: {
//        node {
//          withEnv(['TARGET_ARCH=x64']) {
//            buildElectron()
//          }
//        }
//      },
      winx64: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            destroyVM('win-x64')
            retry(2) {
              try {
                startVM('win-x64')
                setEnvVagrant('win-x64')
                buildElectronVagrant('win-x64', 'npm.cmd')
              } finally {
                destroyVM('win-x64')
              }
            }
          }
        }
      },
      winia32: {
        node {
          withEnv(['TARGET_ARCH=ia32']) {
            destroyVM('win-ia32')
            retry(2) {
              try {
                startVM('win-ia32')
                setEnvVagrant('win-ia32')
                buildElectronVagrant('win-ia32', 'npm.cmd')
              } finally {
                destroyVM('win-ia32')
              }
            }
          }
        }
      },
      linuxx64: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            destroyVM('linux-x64')
            retry(2) {
              try {
                startVM('linux-x64')
                setEnvVagrant('linux-x64')
                installNode('linux-x64')
                setLinuxDisplay('linux-x64')
                buildElectronVagrant('linux-x64')
              } finally {
                destroyVM('linux-x64')
              }
            }
          }
        }
      }
//      linuxia32: {
//        node {
//          withEnv(['TARGET_ARCH=ia32']) {
//            startVM('linux-ia32')
//           try {
//              installNode('linux-ia32')
//              installNode('linux-ia32')
//              buildElectronVagrant('linux-ia32')
//            } finally {
//              destroyVM('linux-ia32')
//            }
//          }
//        }
//      }
    )
  }
}
