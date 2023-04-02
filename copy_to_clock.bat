dotnet build sw -c release
dotnet publish -c release --runtime linux-x64 -o sw\publish --self-contained --framework net5.0-windows sw\writer
pscp -p -r -i c:\users\eric\.ssh\id_rsa_home_devices.ppk sw\publish\* hass@hass.local:/opt/fpgaclock
plink -batch -i c:\users\eric\.ssh\id_rsa_home_devices.PPK hass@hass.local chmod +x /opt/fpgaclock/start