#pragma once
#include <LovyanGFX.hpp>

namespace display {
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7735S _panel_instance;
    lgfx::Bus_SPI _bus_instance;

  public:
    LGFX(int16_t sclk, int16_t mosi, int16_t dc, int16_t cs, int16_t rst, int16_t miso) {
        {
            auto cfg = _bus_instance.config();

            // SPIバスの設定
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 10000000;
            cfg.freq_read = 10000000;
            cfg.spi_3wire = false;
            cfg.use_lock = false;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            cfg.pin_sclk = sclk;
            cfg.pin_mosi = mosi;
            cfg.pin_miso = miso;
            cfg.pin_dc = dc;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs = cs;
            cfg.pin_rst = rst;
            cfg.pin_busy = -1;

            cfg.panel_width = 128;
            cfg.panel_height = 128;
            cfg.offset_x = 2;
            cfg.offset_y = -3;
            cfg.offset_rotation = 3;
            // cfg.dummy_read_pixel = 8;  // ピクセル読出し前のダミーリードのビット数
            // cfg.dummy_read_bits = 1;   // ピクセル以外のデータ読出し前のダミーリードのビット数
            cfg.readable = false;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;

            // 以下はST7735やILI9163のようにピクセル数が可変のドライバで表示がずれる場合にのみ設定してください。
            cfg.memory_width = 128;   // ドライバICがサポートしている最大の幅
            cfg.memory_height = 128;  // ドライバICがサポートしている最大の高さ

            _panel_instance.config(cfg);
        }

        setPanel(&_panel_instance);
    }
};
}  // namespace display