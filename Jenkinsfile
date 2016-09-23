def electronVagrantfilePath = '/Users/Shared/Jenkins/vagrant/electron-vagrant'

def startVM(vagrantfilePath, name) {
  stage("Create ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant up ${name}"
    }
  }
}

def stopVM(vagrantfilePath, name) {
  stage("Stop ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant halt ${name}"
    }
  }
}

def destroyVM(vagrantfilePath, name) {
  stage("Destroy ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant destroy -f ${name}"
    }
  }
}

def vmSSH(vagrantFilePath, name, command) {
  withEnv(["VAGRANT_DOTFILE_PATH=${env.BUILD_TAG}", "VAGRANT_CWD=${vagrantfilePath}"]) {
    sh "vagrant ssh -c ${command}"
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

def installNode() {
  stage('install node') {
    vmSSH(electronVagrantfilePath, 'linux-x64', "'curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -'")
    vmSSH(electronVagrantfilePath, 'linux-x64', "'sudo apt-get install -y nodejs'")
  }
}

timestamps {
  withEnv([
    'ELECTRON_S3_BUCKET=brave-laptop-binaries',
    'LIBCHROMIUMCONTENT_MIRROR=https://s3.amazonaws.com/brave-laptop-binaries/libchromiumcontent']) {
    // LIBCHROMIUMCONTENT_COMMIT - get from previous job

    parallel (
      mac: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            buildElectron()
          }
        }
      },
      winx64: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            startVM(electronVagrantfilePath, 'win-x64')
            buildElectron()
            destroyVM(electronVagrantfilePath, 'win-x64')
          }
        }
      },
      winia32: {
        node {
          withEnv(['TARGET_ARCH=ia32']) {
            startVM(electronVagrantfilePath, 'win-ia32')
            buildElectron()
            destroyVM(electronVagrantfilePath, 'win-ia32')
          }
        }
      },
      linuxx64: {
        node {
          withEnv(['TARGET_ARCH=x64']) {
            startVM(electronVagrantfilePath, 'linux-x64')
            installNode()
            buildElectron()
            destroyVM(electronVagrantfilePath, 'linux-x64')
          }
        }
      }
//      linuxia32: {
//        node {
//          withEnv(['TARGET_ARCH=ia32']) {
//            startVM(electronVagrantfilePath, 'linux-ia32')
//            installNode()
//            buildElectron()
//            destroyVM(electronVagrantfilePath, 'linux-ia32')
//          }
//        }
//      }
    )
  }
}
