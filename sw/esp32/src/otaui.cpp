
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "httpclient.h"
#include "otaui.h"
#include "version.h"

#define CHOICE_UPDATE "UPDATE"
#define CHOICE_EXIT "EXIT"
#define CHOICE_RESTART "RESTART"
#define CHOICE_CANCEL "CANCEL"

#define BASEURL "https://whitemagic.it/panelclock/"

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
        ota->readManifest();
        ota->_state = state::idle;
        vTaskDelete(nullptr); 
    }, "ota", 10000, this, 1, nullptr);
}

int OTAUI::interact() 
{
    auto press = _userinput.getKey();

    if (_state == state::readmanifest || _state == state::restarting)
        return 0;

    auto select = handleChoice(press);

    if (_state == state::restartcountdown)
    {
        removeLogs(1);
        auto remainingSeconds = (int)((_restartCountdownStart.msticks() - _system.now().msticks()) / 1000);
        char buf[30];
        if (remainingSeconds == 0)
        {
            select = CHOICE_RESTART;
        }
        sprintf(buf, "%s (%d)", CHOICE_RESTART, remainingSeconds);
        choices({buf});
    }

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

void OTAUI::readManifest()
{
    HTTPClient client;

    progress("read manifest");

    const char * url = BASEURL "manifest.txt";    
    char buf[512];
    memset(buf, 0, sizeof(buf));
    auto result = client.get(url, (uint8_t*)buf, sizeof(buf));
    if (result != 200)
    {
        log("version check failed");
        detail("no manifest found, error = %d", result);
        choices({ CHOICE_EXIT });
        return;
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
    log("current version %s", currentVersion.version());

    if (currentVersion >= _manifestVersion)
    {
        log("already latest version");
        choices({ CHOICE_EXIT });
    }
    else
    {
        log("new version %s available", _manifestVersion.version());
        choices({ CHOICE_UPDATE, CHOICE_EXIT });
    }
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
        choices({ CHOICE_EXIT });
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
                initiateRestart();
                return;
            }
        }
    }

    if (_cancel)
    {
        error("update cancelled");
        choices({CHOICE_EXIT});
    }
    else
    {
        ret = esp_https_ota_finish(ota_handle);
        if (ret == ESP_OK) 
        {
            log("activating firmware");
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            esp_ota_set_boot_partition(update_partition);
        } 
        else 
        {
            log("update failed;");
            error(" %d (%s)", ret, esp_err_to_name(ret));
        }
        initiateRestart();
    }
}

void OTAUI::initiateRestart()
{
    _state = state::restartcountdown;
    _restartCountdownStart = _system.now();
    _restartCountdownStart.addSeconds(10);
    choices({ CHOICE_RESTART });
}

void OTAUI::restart()
{
    _state = state::restarting;
    progress("restarting");
    vTaskDelay(2500 / portTICK_PERIOD_MS);
    esp_restart();
}
