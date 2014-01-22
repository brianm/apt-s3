# -*- mode: ruby -*-
# vi: set ft=ruby :
Vagrant.configure("2") do |config|
  config.vm.box = "precise64"
  config.vm.box_url = "http://cloud-images.ubuntu.com/vagrant/precise/current/precise-server-cloudimg-amd64-vagrant-disk1.box"
  config.vm.synced_folder ".", "/vagrant"

  lang = ENV['LC_CTYPE']
  $script = <<SCRIPT
sudo locale-gen #{lang}
sudo update-locale LANG=#{lang}
cd /vagrant
sudo apt-get -y install debhelper libapt-pkg-dev cdbs libcurl4-openssl-dev
sudo make deb
sudo cp ../apt-transport* .
sudo dpkg -i *.deb
SCRIPT
  config.vm.provision "shell", inline: $script
end