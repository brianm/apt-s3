# -*- mode: ruby -*-
# vi: set ft=ruby :
Vagrant.configure("2") do |config|

  config.vm.define "precise64" do |precise64|
    precise64.vm.box_url = "http://cloud-images.ubuntu.com/vagrant/precise/current/precise-server-cloudimg-amd64-vagrant-disk1.box"
    precise64.vm.box = "precise64"
  end

  config.vm.define "precise32" do |precise32|
    precise32.vm.box = "precise32"
    precise32.vm.box_url = "http://cloud-images.ubuntu.com/vagrant/precise/current/precise-server-cloudimg-i386-vagrant-disk1.box"
  end

  config.vm.synced_folder ".", "/vagrant"

  lang = ENV['LC_CTYPE']
  $script = <<SCRIPT
sudo locale-gen #{lang}
sudo update-locale LANG=#{lang}
cd /vagrant
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt-get -y -f install debhelper libapt-pkg-dev cdbs libcurl4-openssl-dev
sudo make clean
sudo make deb
mkdir build
sudo cp ../apt-transport* build
arch=`facter architecture`
sudo dpkg -i build/*$arch.deb
SCRIPT
  config.vm.provision "shell", inline: $script
end