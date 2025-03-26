
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include <cstring>

#include "diagnostic.h"
#include "httpclient.h"
#include "otaui.h"
#include "version.h"

#define CHOICE_UPDATE "UPDATE"
#define CHOICE_EXIT "EXIT"
#define CHOICE_RESTART "RESTART"
#define CHOICE_CANCEL "CANCEL"

#define BASEURL "https://whitemagic.it/panelclock/"

OTAUI::OTAUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
    : ConsoleBase(appdata, env, sys, userinput)
{
    _automatic = false;
    std::function<void(Setting*)> handler = [&](Setting *_)
    {
        _checkUpdateInterval = _system.settings().SoftwareUpdateInterval() * 60000;
    };
    _system.settings().onChanged(handler);
    starttimer(_checkUpdateTimer);
    handler(nullptr);
}

void OTAUI::init()
{
    ConsoleBase::init();
    _state = state::readmanifest;
    _cancel = false;

    esp_log_level_set("*", ESP_LOG_DEBUG);
    log("Firmware update");

    xTaskCreate([](void*arg) { 
        auto ota = (OTAUI*)arg;
        ota->_state = state::readmanifest;
        ota->readManifest(false);
        ota->_state = state::idle;
        vTaskDelete(nullptr); 
    }, "ota", 10000, this, 1, nullptr);
}

int OTAUI::interact() 
{
    auto press = _userinput.getKeyPress();

    if (_state == state::readmanifest || _state == state::restarting)
        return 0;

    auto select = handleChoice(press);

    if (select == CHOICE_UPDATE)
    {
        xTaskCreate([](void*arg) { 
            auto ota = (OTAUI*)arg;
            ota->_state = state::update;
            ota->updateOverTheAir(); 
            if (ota->_state == state::update)
            {
                ota->_state = state::idle;
            }
            vTaskDelete(nullptr); 
        }, "ota", 10000, this, 1, nullptr);
    }
    else if (select == CHOICE_EXIT)
    {
        return 1;
    }
    else if (select == CHOICE_CANCEL)
    {
        _cancel = true;   
    }
    else if (select == CHOICE_RESTART)
    {
        removeLogs(1);
        xTaskCreate([](void*arg) { 
            ((OTAUI*)arg)->restart();
        }, "restart", 10000, this, 1, nullptr);
    }
    return 0;
}

bool OTAUI::isUpdateAvailable()
{
    if (_checkUpdateInterval == 0)
        return false;
    
    if (!timeout(_checkUpdateTimer, _checkUpdateInterval))
        return false;

    auto updateAvailable = readManifest(true);
    starttimer(_checkUpdateTimer);

    return updateAvailable;
}

bool OTAUI::readManifest(bool silent)
{
    HTTPClient client;
    auto newversion = false;

    if (!silent) progress("read manifest");

    const char * url = BASEURL "manifest.txt";    
    char buf[512];
    memset(buf, 0, sizeof(buf));
    auto result = client.get(url, (uint8_t*)buf, sizeof(buf));
    if (result != 200)
    {
        if (!silent) log("version check failed");
        if (!silent) detail("no manifest found, error = %d", result);
        if (!silent) choices({ CHOICE_EXIT":9" });
        return newversion;
    }

    char *pstart = buf;
    char *pnext = buf;
    while (pnext)
    {
        pstart = pnext;
        pnext = std::strchr(pstart, '\n');
        if (pnext)
        {
            *pnext++ = 0;
        }
        if (sscanf(pstart, "firmware=%s", pstart) == 1)
        {
            _manifestVersion = Version(pstart);
        }
    }

    auto currentVersion = Version::application();
    if (!silent) log("current version %s", currentVersion.version());

    if (currentVersion >= _manifestVersion)
    {
        if (!silent) log("already latest version");
        if (!silent) choices({ CHOICE_EXIT":9" });
    }
    else
    {
        if (!silent) log("new version %s available", _manifestVersion.version());
        if (!silent) choices({ CHOICE_UPDATE":9", CHOICE_EXIT });
        newversion = true;
    }
    return newversion;
}

void OTAUI::updateOverTheAir() 
{
    Diagnostic::printmeminfo();

    const char * url = BASEURL "firmware.bin";    
    log("get firmware");
    esp_http_client_config_t config = { 0 };
    config.url = url;
    config.timeout_ms = 5000;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL,
    config.cert_pem = nullptr;
    config.crt_bundle_attach = esp_crt_bundle_attach;        

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t ota_handle = NULL;
    esp_err_t ret = esp_https_ota_begin(&ota_config, &ota_handle);
    removeLogs(1);

    if (ret != ESP_OK) 
    {
        log("download failed;");
        error(" %d (%s)", ret, esp_err_to_name(ret));
        choices({ CHOICE_EXIT":9" });
        return;
    }

    log("download");
    choices({ CHOICE_CANCEL });
    for (;;)
    {
        ret = esp_https_ota_perform(ota_handle);
        if (_cancel)
        {
            break;
        }
        else if (ret == ESP_ERR_HTTPS_OTA_IN_PROGRESS) 
        {
            removeLogs(2);
            auto loaded = esp_https_ota_get_image_len_read(ota_handle);
            log("download %.3fMB", loaded / 1024.0f / 1024.0f);
            choices({ CHOICE_CANCEL });
            continue;  // Keep downloading and flashing the firmware
        } 
        else 
        {
            removeLogs(1);
            if (ret == ESP_OK) 
            {
                log("download complete.");
                break;
            } 
            else 
            {
                log("download failed;");
                error(" %d (%s)", ret, esp_err_to_name(ret));
                esp_https_ota_abort(ota_handle);
                choices({ CHOICE_RESTART":9" });
                return;
            }
        }
    }

    if (_cancel)
    {
        esp_https_ota_abort(ota_handle);
        error("update cancelled");
        choices({ CHOICE_EXIT":9" });
    }
    else
    {
        ret = esp_https_ota_finish(ota_handle);
        if (ret == ESP_OK) 
        {
            log("activating firmware");
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            esp_ota_set_boot_partition(update_partition);
            choices({ CHOICE_RESTART":9" });
        } 
        else 
        {
            log("update failed;");
            error(" %d (%s)", ret, esp_err_to_name(ret));
            choices({ CHOICE_RESTART":30" });
        }
    }
}

void OTAUI::restart()
{
    _state = state::restarting;
    progress("restarting");
    vTaskDelay(2500 / portTICK_PERIOD_MS);
    esp_restart();
}
