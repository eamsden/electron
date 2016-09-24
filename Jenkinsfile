def startVM(name) {
  lock("create-vm") {
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
    sh "vagrant ssh ${name} -c '${sshEnv()} ${command}'"
  }
}

def setLinuxDisplay(name) {
  vmSSH(name, "export DISPLAY=:99.0")
}

def npmInstall(name, cmd = 'npm') {
  vmSSH(name, "cd electron; ${cmd} install npm@3.3.12")
}

def buildElectron() {
  lock('build-electorn') {
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

def buildElectronVagrant(name, npmCmd = 'npm', env = '') {
  lock("build-electron-${name}") {
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
        vmSSH(name, "cd electron; python script/clean.py")
        npmInstall(name, npmCmd)
        vmSSH(name, "${env} cd electron; python script/bootstrap.py -v --target_arch ${env.TARGET_ARCH}")
      }
    }
    stage('VM Lint') {
      vmSSH(name, "${env} cd electron; python script/cpplint.py")
    }
    stage('VM Build') {
      vmSSH(name, "${env} cd electron; python script/build.py -c R")
    }
    stage('VM Create Dist') {
      vmSSH(name, "${env} cd electron; python script/create-dist.py")
    }
    stage('VM Upload') {
      retry(3) {
        vmSSH(name, "${env} cd electron; python script/upload.py")
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

def sshEnv(name) {
  return (String[]) ['TARGET_ARCH', 'ELECTRON_S3_BUCKET', 'LIBCHROMIUMCONTENT_MIRROR',
    'CI', 'ELECTRON_RELEASE', 'GYP_DEFINES', 'ELECTRON_S3_SECRET_KEY', 'ELECTRON_S3_ACCESS_KEY',
    'ELECTRON_GITHUB_TOKEN'].collect { "${it}=${env[$it]}" }.join(' ')
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
          withEnv(['TARGET_ARCH=x64', 'PLATFORM=win']) {
            try {
              retry(2) {
                destroyVM('win-x64')
                startVM('win-x64')
                setEnvVagrant('win-x64')
                buildElectronVagrant('win-x64', 'npm.cmd', 'PATH=$PATH:/cygdrive/c/Program\ Files\ \(x86\)/Windows\ Kits/10/Debuggers/x64')
              }
            } finally {
              destroyVM('win-x64')
            }
          }
        }
      }
//      winia32: {
//        node {
//          withEnv(['TARGET_ARCH=ia32', 'PLATFORM=win']) {
//            try {
//              retry(2) {
//                destroyVM('win-ia32')
//                startVM('win-ia32')
//                setEnvVagrant('win-ia32')
//                buildElectronVagrant('win-ia32', 'npm.cmd', 'PATH=$PATH:/cygdrive/c/Program\ Files\ \(x86\)/Windows\ Kits/10/Debuggers/x86')
//              }
//            } finally {
//              destroyVM('win-ia32')
//            }
//          }
//        }
//      },
//      linuxx64: {
//        node {
//          withEnv(['TARGET_ARCH=x64']) {
//            destroyVM('linux-x64')
//            retry(2) {
//              try {
//                startVM('linux-x64')
//                setEnvVagrant('linux-x64')
//                installNode('linux-x64')
//                setLinuxDisplay('linux-x64')
//                buildElectronVagrant('linux-x64')
//              } finally {
//                destroyVM('linux-x64')
//              }
//            }
//          }
//        }
//      }
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
