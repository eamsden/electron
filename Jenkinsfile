def startVM(name) {
  stage("Create /Users/Shared/Jenkins/vagrant/electron-vagrant ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant up ${name}"
    }
  }
}

def stopVM(name) {
  stage("Stop /Users/Shared/Jenkins/vagrant/electron-vagrant ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant halt ${name}"
    }
  }
}

def destroyVM(name) {
  stage("Destroy /Users/Shared/Jenkins/vagrant/electron-vagrant ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
      sh "vagrant destroy -f ${name}"
    }
  }
}

def vmSSH(name, command) {
  withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=/Users/Shared/Jenkins/vagrant/electron-vagrant"]) {
    sh "vagrant ssh ${name} -c ${command}"
  }
}

def buildElectron() {
  stage('Clean') {
    deleteDir()
  }
  stage('Checkout') {
    checkout scm
  }
  stage('Bootstrap') {
    retry(3) {
      timeout(30) {
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

def buildElectronVagrant(name) {
  stage('Clean') {
    deleteDir()
  }
  stage('Checkout') {
    vmSSH(name, "'git clone https://github.com/brave/electron.git electron'")
  }
  stage('Bootstrap') {
    retry(3) {
      timeout(30) {
        vmSSH(name, "'source ~/.profile && cd electron && python script/bootstrap.py -v --target_arch ${env.TARGET_ARCH}'")
      }
    }
  }
  stage('Lint') {
    vmSSH(name, "'source ~/.profile && cd electron && python script/cpplint.py'")
  }
  stage('Build') {
    vmSSH(name, "'source ~/.profile && cd electron && python script/build.py -c R'")
  }
  stage('Create Dist') {
    vmSSH(name, "'source ~/.profile && cd electron && python script/create-dist.py'")
  }
  stage('Upload') {
    retry(3) {
      vmSSH(name, "'source ~/.profile && cd electron && python script/upload.py'")
    }
  }
}

def installNode(name) {
  stage('install node') {
    vmSSH(name, "'curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -'")
    vmSSH(name, "'sudo apt-get install -y nodejs'")
  }
}

timestamps {
  withEnv([
    'ELECTRON_S3_BUCKET=brave-laptop-binaries',
    'LIBCHROMIUMCONTENT_MIRROR=https://s3.amazonaws.com/brave-laptop-binaries/libchromiumcontent']) {
    // LIBCHROMIUMCONTENT_COMMIT - get from previous job

    parallel (
//      mac: {
//        node {
//          withEnv(['TARGET_ARCH=x64']) {
//            buildElectron()
 //         }
 //       }
 //     },
      winx64: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            destroyVM('win-x64')
            startVM('win-x64')
            buildElectronVagrant('win-x64')
            destroyVM('win-x64')
          }
        }
      }
//      winia32: {
//        node {
//          withEnv(['TARGET_ARCH=ia32']) {
//            destroyVM('win-ia32')
//            startVM('win-ia32')
//            buildElectronVagrant('win-ia32')
//            destroyVM('win-ia32')
//          }
//        }
//      },
//      linuxx64: {
//        node {
//          withEnv(['TARGET_ARCH=x64']) {
//            destroyVM('linux-x64')
//            startVM('linux-x64')
//            installNode('linux-x64')
//            buildElectronVagrant('linux-x64')
//            destroyVM('linux-x64')
//          }
//        }
//      }
//      linuxia32: {
//        node {
//          withEnv(['TARGET_ARCH=ia32']) {
//            startVM('linux-ia32')
//            installNode('linux-ia32')
//            buildElectronVagrant('linux-ia32')
//            destroyVM('linux-ia32')
//          }
//        }
//      }
    )
  }
}
