def electronVagrantfilePath = '/Users/Shared/Jenkins/vagrant/electron-vagrant'

def startVM(vagrantfilePath, name) {
  stage("Create ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.WORKSPACE}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant up ${name}"
    }
  }
}

def stopVM(vagrantfilePath, name) {
  stage("Stop ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.WORKSPACE}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant halt ${name}"
    }
  }
}

def destroyVM(vagrantfilePath, name) {
  stage("Destroy ${vagrantfilePath} ${name} VM") {
    withEnv(["VAGRANT_DOTFILE_PATH=${env.WORKSPACE}", "VAGRANT_CWD=${vagrantfilePath}"]) {
      sh "vagrant destroy -f ${name}"
    }
  }
}

def vmSSH(vagrantFilePath, name, command) {
  withEnv(["VAGRANT_DOTFILE_PATH=${env.WORKSPACE}", "VAGRANT_CWD=${vagrantfilePath}"]) {
    sh "vagrant ssh -c ${command}"
  }
}

def buildElectron() {
  stage('Clean') {
    deleteDir()
  }
  stage('Checkout') {
    checkout scm
//    checkout([$class: 'GitSCM', branches: [[name: env.BRANCH_NAME]], doGenerateSubmoduleConfigurations: false, extensions: [], submoduleCfg: [], userRemoteConfigs: [[credentialsId: 'db49b2ab-61d6-48e3-8710-cac2adb1cf99', url: 'https://github.com/brave/electron']]])
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
      }
    )
  }
}
