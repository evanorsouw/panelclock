
Install .net5.0 on Ubuntu 18.04 LTS HASS 
https://docs.microsoft.com/en-us/dotnet/core/install/linux-ubuntu#1804- 

wget https://packages.microsoft.com/config/ubuntu/18.04/packages-microsoftprod.deb -O packages-microsoft-prod.deb 

sudo dpkg -i packages-microsoft-prod.deb 

rm packages-microsoft-prod.deb 

sudo apt-get update; \   
sudo apt-get install -y apt-transport-https && \   
sudo apt-get update && \   
sudo apt-get install -y dotnet-sdk-5.0 

Software geinstalleerd in /opt/fpgaclock 

Systemd service aangemaakt op NUC fpgaclock.service. 

Hiervoor 1 start scriptje aan de \opt\fpgaclock toegevoegd 

De rest word voornamelijk door systemd geregeld.  