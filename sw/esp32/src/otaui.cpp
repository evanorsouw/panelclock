
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "otaui.h"
#include "diagnostic.h"

#define CHOICE_GO "GO"
#define CHOICE_EXIT "EXIT"
#define CHOICE_RESTART "RESTART"
#define CHOICE_CANCEL "CANCEL"

void OTAUI::init()
{
    _yTop = 0;
    _lines.clear();
    _OTARunning = false;
    _cancel = false;
    esp_log_level_set("*", ESP_LOG_DEBUG);
    log(linetype::info, "Firmware update");
    choices({ CHOICE_GO, CHOICE_EXIT });
}

void OTAUI::render(Graphics& graphics)
{
    std::lock_guard<std::mutex> lock(_loglock);
    auto font = _appctx.fontSettings();
    for (auto i=0; i<_lines.size(); i++)
    {
        auto yt = _yTop + i * font->height();
        if (yt >= graphics.dy())
            break;
        auto yb = yt + font->height() - 1;
        if (yb < 0)
            continue;

        auto &info = _lines[i];
        if (info.type == linetype::choices)
        {
            auto x = 0;
            for (auto i=0; i<info.line.size(); ++i)
            {
                auto size = font->textsize(info.line[i].c_str());
                if (info.choice == i)
                {
                    graphics.rect(x+1, yt, size.dx, size.dy + font->descend(), Color::darkgreen);
                    graphics.rect(x, yt + 1, size.dx + 2, size.dy - 2 + font->descend(), Color::darkgreen);
                    graphics.text(font, x + 1, yb + font->descend(), info.line[i].c_str(), info.color());
                }
                else
                {
                    graphics.text(font, x + 1, yb + font->descend(), info.line[i].c_str(), info.color());
                }
                x += size.dx + 2;
            }
        }
        else
        {
            auto line = info.line[0].c_str();
            graphics.text(font, -info.scrolloffset, yb + font->descend(), line, info.color());
            auto linedx = font->textsize(line).dx;
            switch (info.state)
            {
            case scrollstate::begin:
                info.scrolloffset = 0;
                if (timeout(info.scrolldelay, 1000))
                {
                    starttimer(info.scrolldelay);
                    info.state = scrollstate::scrolling;
                }
                break;
            case scrollstate::scrolling:
                if (info.scrolloffset + graphics.dx() >= linedx)
                {
                    starttimer(info.scrolldelay);
                    info.state = scrollstate::end;
                }
                else
                {
                    info.scrolloffset++;
                }
                break;
            case scrollstate::end:
                if (timeout(info.scrolldelay, 1500))
                {
                    info.state = scrollstate::begin;
                    starttimer(info.scrolldelay);
                }
                break;
            }
        }
    }
    auto yb = (int)_yTop + _lines.size() * font->height();
    auto yt = yb - font->height();
    auto ytarget = _yTop;
    if (yt < 0)
        ytarget += -yt;
    else if (yb > graphics.dy())
        ytarget -= yb - graphics.dy();

    graduallyUpdateVariable(_yTop, ytarget, 0.5f);
}

int OTAUI::interact() 
{ 
    if (!_OTARunning && _userinput.howLongIsKeyDown(UserInput::KEY_SET) > 1500)
        return 1;   // abort

    if (_lines.back().type != linetype::choices)
        return 0;

    auto press = _userinput.getKey();
    auto &info = _lines.back();
    switch (press.key)
    {
    case UserInput::KEY_SET:
        if (info.line[info.choice] == CHOICE_GO)
        {
            xTaskCreate([](void*arg) { 
                auto ota = (OTAUI*)arg;
                ota->_OTARunning = true;
                ota->updateOverTheAir(); 
                ota->_OTARunning = false;
                vTaskDelete(nullptr); 
            }, "ota", 10000, this, 1, nullptr);
        }
        else if (info.line[info.choice] == CHOICE_EXIT)
        {
            return 1;
        }
        else if (info.line[info.choice] == CHOICE_CANCEL)
        {
            _cancel = true;   
        }
        else if (info.line[info.choice] == CHOICE_RESTART)
        {
            xTaskCreate([](void*arg) { 
                ((OTAUI*)arg)->restart();
            }, "restart", 10000, this, 1, nullptr);
        }
        break;
    case UserInput::KEY_DOWN:
        info.choice = (info.choice + 1) % info.line.size(); 
        break;
    case UserInput::KEY_UP:
        info.choice = (info.choice - 1 + info.line.size()) % info.line.size(); 
        break;
    }        
    return 0;
}

void OTAUI::updateOverTheAir() 
{
    Diagnostic::printmeminfo();

    const char * url = "https://whitemagic.it/panelclock/panelclock-image.bin";    
    log(linetype::info, "get firmware");
    log(linetype::detail, "%s", url);
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
    if (ret != ESP_OK) 
    {
        log(linetype::info, "OTA begin failed;");
        log(linetype::error, " %d (%s)", ret, esp_err_to_name(ret));
        return;
    }

    log(linetype::info, "download");
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
            log(linetype::info, "download %.3fMB", loaded / 1024.0f / 1024.0f);
            choices({ CHOICE_CANCEL });
            continue;  // Keep downloading and flashing the firmware
        } 
        else if (ret == ESP_OK) 
        {
            log(linetype::info, "download complete.");
            break;
        } 
        else 
        {
            log(linetype::info, "download failed;");
            log(linetype::error, " %d (%s)", ret, esp_err_to_name(ret));
            esp_https_ota_abort(ota_handle);
            return;
        }
    }

    if (_cancel)
    {
        log(linetype::error, "update cancelled");
    }
    else
    {
        ret = esp_https_ota_finish(ota_handle);
        if (ret == ESP_OK) 
        {
            log(linetype::info, "set boot partition");
            const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
            esp_ota_set_boot_partition(update_partition);
        } 
        else 
        {
            log(linetype::info, "update failed;");
            log(linetype::error, " %d (%s)", ret, esp_err_to_name(ret));
        }
    }
    choices({ CHOICE_RESTART });
}

void OTAUI::restart()
{
    log(linetype::info, "restarting");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}

void OTAUI::removeLogs(int n)
{
    std::lock_guard<std::mutex> lock(_loglock);
    while (n-- > 0)
    {
        if (!_lines.empty())
        {
            _lines.pop_back();
        }
    }
}

void OTAUI::log(linetype type, const char *fmt, ...)
{
    std::lock_guard<std::mutex> lock(_loglock);
    va_list argp;
    va_start(argp, fmt);
    char buf[512];
    vsnprintf(buf, sizeof(buf), fmt, argp);
    _lines.push_back(lineinfo(std::vector<std::string>{ buf }, type));
}

void OTAUI::choices(std::initializer_list<std::string> list)
{
    _lines.push_back(lineinfo(list, linetype::choices));
}
