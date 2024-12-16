
#include <algorithm>

#include <nvs.h>
#include <nvs_flash.h>

#include "settings.h"

bool Settings::loadSettings()
{    
    nvs_handle handle;

    auto result = nvs_open("standalonepanel", NVS_READWRITE, &handle);
    if (result != ESP_OK)
    {
        printf("error opening nvs: %s\n", esp_err_to_name(result));
        return false;
    }

    nvs_iterator_t it;
    result = nvs_entry_find("nvs", "standalonepanel", NVS_TYPE_ANY, &it);
    while (result == ESP_OK)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        char buf[1024];
        auto bufsize = sizeof(buf);
        nvs_get_str(handle, info.key, buf, &bufsize);
        printf("found setting '%s':'%s'\n", info.key, buf);
        add(info.key, buf);
        result = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    nvs_close(handle);
    return true;
}

bool Settings::saveSettings()
{    
    nvs_handle handle;

    auto result = nvs_open("standalonepanel", NVS_READWRITE, &handle);
    if (result != ESP_OK)
    {
        printf("error opening nvs: %s\n", esp_err_to_name(result));
        return false;
    }

    for(auto setting : _settings)
    {
        auto name = setting.first.c_str();
        auto value = setting.second->asstring();
        printf("write setting '%s':'%s'\n", name, value);
        nvs_set_str(handle, name, value);
    }
    nvs_commit(handle);
    nvs_close(handle);

    return true;
}

Setting *Settings::add(const char *name, const char *value)
{
    Setting *setting;
    auto it = std::find_if(_settings.begin(), _settings.end(), [&](std::pair<std::string, Setting*> kv) { return kv.first == name; });
    if (it == _settings.end())
    {
        setting = new Setting(name, value);
        _settings[name] = setting;            
    }
    else
    {
        setting = it->second;
        setting->set(value);
    }
    return setting;
}    

Setting *Settings::get(const char *name) const 
{ 
    auto it = _settings.find(name);
    if (it == _settings.end())
    {
        printf("setting='%s' not found\n", name);
        abort();
    }
    return it->second;
}

Setting *Settings::get(const char *name, const char *defaultValue) 
{ 
    auto it = _settings.find(name);
    if (it == _settings.end())
        return add(name, defaultValue);
    return it->second;
}

