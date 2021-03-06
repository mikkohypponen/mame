// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for early Williams games

**************************************************************************/


#include "cpu/m6809/m6809.h"
#include "cpu/m6800/m6800.h"
#include "sound/hc55516.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/6821pia.h"
#include "machine/bankdev.h"
#include "audio/williams.h"
#include "emupal.h"
#include "screen.h"

class williams_state : public driver_device
{
public:
	williams_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_nvram(*this, "nvram"),
		m_videoram(*this, "videoram"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_bankc000(*this, "bankc000"),
		m_watchdog(*this, "watchdog"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_pia(*this, "pia_%u", 0U) { }

	enum
	{
		//controlbyte (0xCA00) bit definitions
		WMS_BLITTER_CONTROLBYTE_NO_EVEN = 0x80,
		WMS_BLITTER_CONTROLBYTE_NO_ODD = 0x40,
		WMS_BLITTER_CONTROLBYTE_SHIFT = 0x20,
		WMS_BLITTER_CONTROLBYTE_SOLID = 0x10,
		WMS_BLITTER_CONTROLBYTE_FOREGROUND_ONLY = 0x08,
		WMS_BLITTER_CONTROLBYTE_SLOW = 0x04, //2us blits instead of 1us
		WMS_BLITTER_CONTROLBYTE_DST_STRIDE_256 = 0x02,
		WMS_BLITTER_CONTROLBYTE_SRC_STRIDE_256 = 0x01
	};

	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_memory_bank m_mainbank;
	uint8_t *m_mayday_protection;
	uint8_t m_blitter_config;
	uint16_t m_blitter_clip_address;
	uint8_t m_blitter_window_enable;
	uint8_t m_cocktail;
	std::unique_ptr<rgb_t[]> m_palette_lookup;
	uint8_t m_blitterram[8];
	uint8_t m_blitter_xor;
	uint8_t m_blitter_remap_index;
	const uint8_t *m_blitter_remap;
	std::unique_ptr<uint8_t[]> m_blitter_remap_lookup;
	DECLARE_WRITE8_MEMBER(williams_vram_select_w);
	DECLARE_WRITE8_MEMBER(williams_cmos_w);
	DECLARE_WRITE8_MEMBER(bubbles_cmos_w);
	DECLARE_WRITE8_MEMBER(williams_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(defender_video_control_w);
	DECLARE_WRITE8_MEMBER(defender_bank_select_w);
	DECLARE_READ8_MEMBER(mayday_protection_r);
	DECLARE_WRITE8_MEMBER(sinistar_vram_select_w);
	DECLARE_READ8_MEMBER(williams_video_counter_r);
	DECLARE_WRITE8_MEMBER(williams_blitter_w);
	void init_sinistar();
	void init_stargate();
	void init_playball();
	void init_defender();
	void init_mayday();
	void init_lottofun();
	void init_alienaru();
	void init_defndjeu();
	void init_splat();
	void init_joust();
	void init_alienar();
	void init_robotron();
	void init_bubbles();
	DECLARE_MACHINE_START(defender);
	DECLARE_MACHINE_RESET(defender);
	DECLARE_VIDEO_START(williams);
	DECLARE_MACHINE_START(williams);
	DECLARE_MACHINE_START(williams_common);
	uint32_t screen_update_williams(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(williams_deferred_snd_cmd_w);
	TIMER_DEVICE_CALLBACK_MEMBER(williams_va11_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(williams_count240_callback);
	DECLARE_WRITE8_MEMBER(williams_snd_cmd_w);
	DECLARE_WRITE8_MEMBER(playball_snd_cmd_w);
	DECLARE_READ8_MEMBER(williams_49way_port_0_r);
	DECLARE_WRITE_LINE_MEMBER(lottofun_coin_lock_w);
	DECLARE_PALETTE_INIT(williams);

	void state_save_register();
	void blitter_init(int blitter_config, const uint8_t *remap_prom);
	inline void blit_pixel(address_space &space, int dstaddr, int srcdata, int controlbyte);
	int blitter_core(address_space &space, int sstart, int dstart, int w, int h, int data);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	optional_device<address_map_bank_device> m_bankc000;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_paletteram;
	optional_device_array<pia6821_device, 4> m_pia;
	void playball(machine_config &config);
	void defender(machine_config &config);
	void sinistar(machine_config &config);
	void lottofun(machine_config &config);
	void williams(machine_config &config);
	void williams_muxed(machine_config &config);
	void jin(machine_config &config);
	void defender_bankc000_map(address_map &map);
	void defender_map(address_map &map);
	void defender_sound_map(address_map &map);
	void sinistar_map(address_map &map);
	void sound_map(address_map &map);
	void williams_map(address_map &map);
};

class spdball_state : public williams_state
{
public:
	spdball_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag) { }

	void driver_init() override;

	void spdball(machine_config &config);
	void spdball_map(address_map &map);
};

class blaster_state : public williams_state
{
public:
	blaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams_state(mconfig, type, tag),
		m_soundcpu_b(*this, "soundcpu_b"),
		m_blaster_palette_0(*this, "blaster_pal0"),
		m_blaster_scanline_control(*this, "blaster_scan"),
		m_blaster_bankb(*this, "blaster_bankb") { }

	optional_device<cpu_device> m_soundcpu_b;
	required_shared_ptr<uint8_t> m_blaster_palette_0;
	required_shared_ptr<uint8_t> m_blaster_scanline_control;
	optional_memory_bank m_blaster_bankb;

	rgb_t m_blaster_color0;
	uint8_t m_blaster_video_control;
	uint8_t m_vram_bank;
	uint8_t m_rom_bank;

	DECLARE_WRITE8_MEMBER(blaster_vram_select_w);
	DECLARE_WRITE8_MEMBER(blaster_bank_select_w);
	DECLARE_WRITE8_MEMBER(blaster_remap_select_w);
	DECLARE_WRITE8_MEMBER(blaster_video_control_w);
	TIMER_CALLBACK_MEMBER(blaster_deferred_snd_cmd_w);
	DECLARE_WRITE8_MEMBER(blaster_snd_cmd_w);

	void init_blaster();
	DECLARE_MACHINE_START(blaster);
	DECLARE_VIDEO_START(blaster);
	uint32_t screen_update_blaster(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	inline void update_blaster_banking();
	void blastkit(machine_config &config);
	void blaster(machine_config &config);
	void blaster_map(address_map &map);
	void sound_map_b(address_map &map);
};


class williams2_state : public williams_state
{
public:
	williams2_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams_state(mconfig, type, tag),
		m_bank8000(*this, "bank8000"),
		m_gfxdecode(*this, "gfxdecode"),
		m_williams2_tileram(*this, "williams2_tile") { }

	required_device<address_map_bank_device> m_bank8000;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_williams2_tileram;

	tilemap_t *m_bg_tilemap;
	uint16_t m_tilemap_xscroll;
	uint8_t m_williams2_fg_color;
	uint8_t m_williams2_tilemap_config;

	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_WRITE8_MEMBER(williams2_bank_select_w);
	DECLARE_WRITE8_MEMBER(williams2_watchdog_reset_w);
	DECLARE_WRITE8_MEMBER(williams2_7segment_w);
	DECLARE_WRITE8_MEMBER(williams2_paletteram_w);
	DECLARE_WRITE8_MEMBER(williams2_fg_select_w);
	DECLARE_WRITE8_MEMBER(williams2_bg_select_w);
	DECLARE_WRITE8_MEMBER(williams2_tileram_w);
	DECLARE_WRITE8_MEMBER(williams2_xscroll_low_w);
	DECLARE_WRITE8_MEMBER(williams2_xscroll_high_w);
	DECLARE_WRITE8_MEMBER(williams2_blit_window_enable_w);
	TIMER_DEVICE_CALLBACK_MEMBER(williams2_va11_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(williams2_endscreen_callback);
	TIMER_CALLBACK_MEMBER(williams2_deferred_snd_cmd_w);
	DECLARE_WRITE8_MEMBER(williams2_snd_cmd_w);

	void init_mysticm();
	void init_tshoot();
	void init_inferno();
	DECLARE_MACHINE_START(williams2);
	DECLARE_MACHINE_RESET(williams2);
	DECLARE_VIDEO_START(williams2);
	uint32_t screen_update_williams2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void williams2(machine_config &config);
	void mysticm(machine_config &config);
	void inferno(machine_config &config);
	void williams2_bank8000_map(address_map &map);
	void williams2_common_map(address_map &map);
	void williams2_d000_ram_map(address_map &map);
	void williams2_d000_rom_map(address_map &map);
	void williams2_sound_map(address_map &map);
};


class tshoot_state : public williams2_state
{
public:
	tshoot_state(const machine_config &mconfig, device_type type, const char *tag) :
		williams2_state(mconfig, type, tag),
		m_gun(*this, {"GUNX", "GUNY"}),
		m_grenade_lamp(*this, "Grenade_lamp"),
		m_gun_lamp(*this, "Gun_lamp"),
		m_p1_gun_recoil(*this, "Player1_Gun_Recoil"),
		m_feather_blower(*this, "Feather_Blower") { }

	DECLARE_CUSTOM_INPUT_MEMBER(gun_r);
	DECLARE_WRITE_LINE_MEMBER(maxvol_w);
	DECLARE_WRITE8_MEMBER(lamp_w);

	DECLARE_MACHINE_START(tshoot);

	void tshoot(machine_config &config);
private:
	required_ioport_array<2> m_gun;
	output_finder<> m_grenade_lamp;
	output_finder<> m_gun_lamp;
	output_finder<> m_p1_gun_recoil;
	output_finder<> m_feather_blower;
};

class joust2_state : public williams2_state
{
public:
	joust2_state(const machine_config &mconfig, device_type type, const char *tag)
		: williams2_state(mconfig, type, tag),
		m_cvsd_sound(*this, "cvsd") { }

	required_device<williams_cvsd_sound_device> m_cvsd_sound;
	uint16_t m_joust2_current_sound_data;

	void init_joust2();
	DECLARE_MACHINE_START(joust2);
	DECLARE_MACHINE_RESET(joust2);
	TIMER_CALLBACK_MEMBER(joust2_deferred_snd_cmd_w);
	DECLARE_WRITE8_MEMBER(joust2_snd_cmd_w);
	DECLARE_WRITE_LINE_MEMBER(joust2_pia_3_cb1_w);
	void joust2(machine_config &config);
};

/*----------- defined in video/williams.cpp -----------*/

#define WILLIAMS_BLITTER_NONE       0       /* no blitter */
#define WILLIAMS_BLITTER_SC1        1       /* Special Chip 1 blitter */
#define WILLIAMS_BLITTER_SC2        2       /* Special Chip 2 "bugfixed" blitter */

#define WILLIAMS_TILEMAP_MYSTICM    0       /* IC79 is a 74LS85 comparator */
#define WILLIAMS_TILEMAP_TSHOOT     1       /* IC79 is a 74LS157 selector jumpered to be enabled */
#define WILLIAMS_TILEMAP_JOUST2     2       /* IC79 is a 74LS157 selector jumpered to be disabled */
