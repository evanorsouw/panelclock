
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

OTAUI::OTAUI(ApplicationContext &appdata, IEnvironment &env, System &sys, UserInput &userinput)
    : ConsoleBase(appdata, env, sys, userinput)
{
    _automatic = false;
    std::function<void(Setting*)> handler = [&](Setting *_)
    {
        _checkUpdateInterval = _system.settings().SoftwareUpdateInterval();
    };
    _system.settings().onChanged(handler);
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

    // always check on exact minute/hour/day etc boundaries for predictability
    auto now = _system.now();    
    if (now.sec() != 0)
        return false;

    auto mins = now.hour() * 60 + now.min();
    if ((mins % _checkUpdateInterval) != 0)
        return false;
        
    auto updateAvailable = readManifest(true);

    // prevent double checks in 1 second
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    return updateAvailable;
}

bool OTAUI::current_firmware_needs_acceptance()
{
    esp_ota_img_states_t state;
    auto result = esp_ota_get_state_partition(esp_ota_get_running_partition(), &state);
    return result == ESP_OK && state == ESP_OTA_IMG_PENDING_VERIFY;
}

void OTAUI::accept_current_firmware()
{
    printf("current software version accepted\n");
    esp_ota_mark_app_valid_cancel_rollback();
}

bool OTAUI::readManifest(bool silent)
{
    HTTPClient client;
    auto newversion = false;

    if (!silent) progress("read manifest");

    auto url = _system.settings().SoftwareUpdateURL() + "/manifest.txt";    
    char buf[512];
    memset(buf, 0, sizeof(buf));
    auto result = client.get(url.c_str(), (uint8_t*)buf, sizeof(buf));
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
    if (!silent) log("current version %s", currentVersion.astxt());

    if (currentVersion >= _manifestVersion)
    {
        if (!silent) log("already latest version");
        if (!silent) choices({ CHOICE_EXIT":9" });
    }
    else
    {
        if (!silent) log("new version %s available", _manifestVersion.astxt());
        if (!silent) choices({ _automatic ? CHOICE_UPDATE":1" : CHOICE_UPDATE":9", CHOICE_EXIT });
        newversion = true;
    }
    return newversion;
}

void OTAUI::updateOverTheAir() 
{
    Diagnostic::printmeminfo();

    auto url = _system.settings().SoftwareUpdateURL() + "/firmware.bin";    
    log("get firmware");
    esp_http_client_config_t config = { 0 };
    config.url = url.c_str();
    config.timeout_ms = 5000;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL,
    config.cert_pem = nullptr;
    config.crt_bundle_attach = esp_crt_bundle_attach;        

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    _downloaded = 0;
    _progress = 0.0f;
    esp_https_ota_handle_t ota_handle = NULL;
    accept_current_firmware();
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
            auto total = esp_https_ota_get_image_size(ota_handle);
            _downloaded = esp_https_ota_get_image_len_read(ota_handle); 
            _progress = _downloaded * 100.0f / total;
            log("download %d%%", _progress);
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
            choices({ _automatic ? CHOICE_RESTART":1" : CHOICE_RESTART":9" });
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
    vTaskDelay((_automatic ? 500 : 2500) / portTICK_PERIOD_MS);
    esp_restart();
}
