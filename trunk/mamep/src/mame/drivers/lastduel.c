/**************************************************************************

  Last Duel                       - Capcom, 1988
  LED Storm                       - Capcom, 1988
  Mad Gear                        - Capcom, 1989

  Emulation by Bryan McPhail, mish@tendril.co.uk

  Trivia ;)  The Mad Gear pcb has an unused pad on the board for an i8751
microcontroller.

TODO:
- The seem to be minor priority issues in Mad Gear, but the game might just
  be like that. The priority PROM is missing.
- visible area might be wrong

**************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"

WRITE16_HANDLER( lastduel_vram_w );
WRITE16_HANDLER( lastduel_flip_w );
WRITE16_HANDLER( lastduel_scroll1_w );
WRITE16_HANDLER( lastduel_scroll2_w );
WRITE16_HANDLER( madgear_scroll1_w );
WRITE16_HANDLER( madgear_scroll2_w );
WRITE16_HANDLER( lastduel_scroll_w );
WRITE16_HANDLER( lastduel_palette_word_w );
VIDEO_START( lastduel );
VIDEO_START( madgear );
VIDEO_UPDATE( lastduel );
VIDEO_UPDATE( madgear );
VIDEO_EOF( lastduel );

extern UINT16 *lastduel_vram,*lastduel_scroll2,*lastduel_scroll1;

/******************************************************************************/

static WRITE16_HANDLER( lastduel_sound_w )
{
	if (ACCESSING_BITS_0_7)
		soundlatch_w(space,0,data & 0xff);
}

/******************************************************************************/

static ADDRESS_MAP_START( lastduel_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_READ(SMH_ROM)
	AM_RANGE(0xfc0800, 0xfc0fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfc4000, 0xfc4001) AM_READ_PORT("P1_P2")
	AM_RANGE(0xfc4002, 0xfc4003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfc4004, 0xfc4005) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc4006, 0xfc4007) AM_READ_PORT("DSW2")
	AM_RANGE(0xfcc000, 0xfcdfff) AM_READ(SMH_RAM)
	AM_RANGE(0xfd0000, 0xfd3fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfd4000, 0xfd7fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfd8000, 0xfd87ff) AM_READ(SMH_RAM)
	AM_RANGE(0xfe0000, 0xffffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lastduel_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xfc0000, 0xfc0003) AM_WRITE(SMH_NOP) /* Written rarely */
	AM_RANGE(0xfc0800, 0xfc0fff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0xfc4000, 0xfc4001) AM_WRITE(lastduel_flip_w)
	AM_RANGE(0xfc4002, 0xfc4003) AM_WRITE(lastduel_sound_w)
	AM_RANGE(0xfc8000, 0xfc800f) AM_WRITE(lastduel_scroll_w)
	AM_RANGE(0xfcc000, 0xfcdfff) AM_WRITE(lastduel_vram_w) AM_BASE(&lastduel_vram)
	AM_RANGE(0xfd0000, 0xfd3fff) AM_WRITE(lastduel_scroll1_w) AM_BASE(&lastduel_scroll1)
	AM_RANGE(0xfd4000, 0xfd7fff) AM_WRITE(lastduel_scroll2_w) AM_BASE(&lastduel_scroll2)
	AM_RANGE(0xfd8000, 0xfd87ff) AM_WRITE(lastduel_palette_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xfe0000, 0xffffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( madgear_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(SMH_ROM)
	AM_RANGE(0xfc1800, 0xfc1fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfc4000, 0xfc4001) AM_READ_PORT("DSW1")
	AM_RANGE(0xfc4002, 0xfc4003) AM_READ_PORT("DSW2")
	AM_RANGE(0xfc4004, 0xfc4005) AM_READ_PORT("P1_P2")
	AM_RANGE(0xfc4006, 0xfc4007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfc8000, 0xfc9fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfcc000, 0xfcc7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xfd4000, 0xfd7fff) AM_READ(SMH_RAM)
	AM_RANGE(0xfd8000, 0xfdffff) AM_READ(SMH_RAM)
	AM_RANGE(0xff0000, 0xffffff) AM_READ(SMH_RAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( madgear_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xfc1800, 0xfc1fff) AM_WRITE(SMH_RAM) AM_BASE(&spriteram16) AM_SIZE(&spriteram_size)
	AM_RANGE(0xfc4000, 0xfc4001) AM_WRITE(lastduel_flip_w)
	AM_RANGE(0xfc4002, 0xfc4003) AM_WRITE(lastduel_sound_w)
	AM_RANGE(0xfc8000, 0xfc9fff) AM_WRITE(lastduel_vram_w) AM_BASE(&lastduel_vram)
	AM_RANGE(0xfcc000, 0xfcc7ff) AM_WRITE(lastduel_palette_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xfd0000, 0xfd000f) AM_WRITE(lastduel_scroll_w)
	AM_RANGE(0xfd4000, 0xfd7fff) AM_WRITE(madgear_scroll1_w) AM_BASE(&lastduel_scroll1)
	AM_RANGE(0xfd8000, 0xfdffff) AM_WRITE(madgear_scroll2_w) AM_BASE(&lastduel_scroll2)
	AM_RANGE(0xff0000, 0xffffff) AM_WRITE(SMH_RAM)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_READ(SMH_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xe800, 0xe801) AM_DEVREAD(SOUND, "ym1", ym2203_r)
	AM_RANGE(0xf000, 0xf001) AM_DEVREAD(SOUND, "ym2", ym2203_r)
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xe800, 0xe801) AM_DEVWRITE(SOUND, "ym1", ym2203_w)
	AM_RANGE(0xf000, 0xf001) AM_DEVWRITE(SOUND, "ym2", ym2203_w)
ADDRESS_MAP_END

static WRITE8_HANDLER( mg_bankswitch_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "audiocpu");

	bankaddress = 0x10000 + (data & 0x01) * 0x4000;
	memory_set_bankptr(space->machine, 3,&RAM[bankaddress]);
}

static ADDRESS_MAP_START( mg_sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)
	AM_RANGE(0x8000, 0xcfff) AM_READ(SMH_BANK3)
	AM_RANGE(0xd000, 0xd7ff) AM_READ(SMH_RAM)
	AM_RANGE(0xf000, 0xf001) AM_DEVREAD(SOUND, "ym1", ym2203_r)
	AM_RANGE(0xf002, 0xf003) AM_DEVREAD(SOUND, "ym2", ym2203_r)
	AM_RANGE(0xf006, 0xf006) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mg_sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xcfff) AM_WRITE(SMH_ROM)
	AM_RANGE(0xd000, 0xd7ff) AM_WRITE(SMH_RAM)
	AM_RANGE(0xf000, 0xf001) AM_DEVWRITE(SOUND, "ym1", ym2203_w)
	AM_RANGE(0xf002, 0xf003) AM_DEVWRITE(SOUND, "ym2", ym2203_w)
	AM_RANGE(0xf004, 0xf004) AM_DEVWRITE(SOUND, "oki", okim6295_w)
	AM_RANGE(0xf00a, 0xf00a) AM_WRITE(mg_bankswitch_w)
ADDRESS_MAP_END

/******************************************************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*16
};

static const gfx_layout madgear_tile =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 3*4, 2*4, 1*4, 0*4 },
	{ 0, 1, 2, 3, 16+0, 16+1, 16+2, 16+3,
			32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+16+0, 32*16+16+1, 32*16+16+2, 32*16+16+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*16
};

static const gfx_layout madgear_tile2 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 1*4, 3*4, 0*4, 2*4 },
	{ 0, 1, 2, 3, 16+0, 16+1, 16+2, 16+3,
			32*16+0, 32*16+1, 32*16+2, 32*16+3, 32*16+16+0, 32*16+16+1, 32*16+16+2, 32*16+16+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*16
};

static GFXDECODE_START( lastduel )
	GFXDECODE_ENTRY( "gfx1", 0,sprite_layout, 0x200, 16 )	/* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx2", 0,text_layout,   0x300, 16 )	/* colors 0x300-0x33f */
	GFXDECODE_ENTRY( "gfx3", 0,tile_layout,   0x000, 16 )	/* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx4", 0,tile_layout,   0x100, 16 )	/* colors 0x100-0x1ff */
GFXDECODE_END

static GFXDECODE_START( madgear )
	GFXDECODE_ENTRY( "gfx1", 0,sprite_layout, 0x200, 16 )	/* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx2", 0,text_layout,   0x300, 16 )	/* colors 0x300-0x33f */
	GFXDECODE_ENTRY( "gfx3", 0,madgear_tile,  0x000, 16 )	/* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx4", 0,madgear_tile2, 0x100, 16 )	/* colors 0x100-0x1ff */
GFXDECODE_END

/******************************************************************************/

/* handler called by the 2203 emulator when the internal timers cause an IRQ */
static void irqhandler(const device_config *device, int irq)
{
	cpu_set_input_line(device->machine->cpu[1],0,irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_config =
{
	{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	},
	irqhandler
};

static INTERRUPT_GEN( lastduel_interrupt )
{
	if (cpu_getiloops(device) == 0)
		cpu_set_input_line(device, 2, HOLD_LINE); /* VBL */
	else
		cpu_set_input_line(device, 4, HOLD_LINE); /* Controls */
}

static INTERRUPT_GEN( madgear_interrupt )
{
	if (cpu_getiloops(device) == 0)
		cpu_set_input_line(device, 5, HOLD_LINE); /* VBL */
	else
		cpu_set_input_line(device, 6, HOLD_LINE); /* Controls */
}

static MACHINE_DRIVER_START( lastduel )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000) /* Could be 8 MHz */
	MDRV_CPU_PROGRAM_MAP(lastduel_readmem,lastduel_writemem)
	MDRV_CPU_VBLANK_INT_HACK(lastduel_interrupt,3)	/* 1 for vbl, 2 for control reads?? */

	MDRV_CPU_ADD("audiocpu", Z80, 3579545) /* Accurate */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK | VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 1*8, 31*8-1 )

	MDRV_GFXDECODE(lastduel)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(lastduel)
	MDRV_VIDEO_EOF(lastduel)
	MDRV_VIDEO_UPDATE(lastduel)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 3579545)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("ym2", YM2203, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( madgear )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000) /* Accurate */
	MDRV_CPU_PROGRAM_MAP(madgear_readmem,madgear_writemem)
	MDRV_CPU_VBLANK_INT_HACK(madgear_interrupt,3)	/* 1 for vbl, 2 for control reads?? */

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(mg_sound_readmem,mg_sound_writemem)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK | VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 1*8, 31*8-1 )

	MDRV_GFXDECODE(madgear)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(madgear)
	MDRV_VIDEO_EOF(lastduel)
	MDRV_VIDEO_UPDATE(madgear)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("ym2", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_10MHz/10)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.98)
MACHINE_DRIVER_END

/******************************************************************************/

static INPUT_PORTS_START( lastduel )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, "Difficult" )
	PORT_DIPSETTING(      0x0000, "Very Difficult" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )	/* Could be cabinet type? */
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x2000, "20000 60000 80000" )
	PORT_DIPSETTING(      0x3000, "30000 80000 80000" )
	PORT_DIPSETTING(      0x1000, "40000 80000 80000" )
	PORT_DIPSETTING(      0x0000, "40000 80000 100000" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x04, 0x04, "Type" )
	PORT_DIPSETTING(    0x04, "Car" )
	PORT_DIPSETTING(    0x00, "Plane" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( madgear )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, "Difficult" )
	PORT_DIPSETTING(      0x0000, "Very Difficult" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0030, "Upright One Player" )
	PORT_DIPSETTING(      0x0000, "Upright Two Players" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Cocktail ) )
/*  PORT_DIPSETTING(      0x0020, "Upright One Player" ) */
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Background Music" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("DSW2") /* Free play is COIN B all off, COIN A all on */
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("SYSTEM")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

/******************************************************************************/

ROM_START( lastduel )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "ldu-06.rom",   0x00000, 0x20000, CRC(4228a00b) SHA1(8c23f74f682ba2074da9f3306600c881ce41e50f) )
	ROM_LOAD16_BYTE( "ldu-05.rom",   0x00001, 0x20000, CRC(7260434f) SHA1(55eeb12977efb3c6afd86d68612782ba526c9055) )
	ROM_LOAD16_BYTE( "ldu-04.rom",   0x40000, 0x10000, CRC(429fb964) SHA1(78769b05e62c190d846dd08214427d1abbbe2bba) )
	ROM_LOAD16_BYTE( "ldu-03.rom",   0x40001, 0x10000, CRC(5aa4df72) SHA1(9e7315b793f09c8b422bad1ce776588e3a48d80c) )

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "ld_02.bin",    0x0000, 0x10000, CRC(91834d0c) SHA1(aaa63b8470fc19b82c25028ab27675a7837ab9a1) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_09.bin",    0x000000, 0x10000, CRC(f8fd5243) SHA1(fad80d8959f50a83eb2e47788a8183284d19bea6) ) /* sprites */
	ROM_LOAD( "ld_10.bin",    0x010000, 0x10000, CRC(b49ad746) SHA1(4e609982d60155b0df13a156c37bdf2a25626632) )
	ROM_LOAD( "ld_11.bin",    0x020000, 0x10000, CRC(1a0d180e) SHA1(a68a7f5d00da99a8068876fd2d61c726047aca80) )
	ROM_LOAD( "ld_12.bin",    0x030000, 0x10000, CRC(b2745e26) SHA1(b511631fe4e21f3d2dc7440b3f69cd5edb43d20e) )
	ROM_LOAD( "ld_15.bin",    0x040000, 0x10000, CRC(96b13bbc) SHA1(f2df8d4f11e9192063063ff2e9e4fe76971c5b24) )
	ROM_LOAD( "ld_16.bin",    0x050000, 0x10000, CRC(9d80f7e6) SHA1(ce7c10eba6a9f6a1fad655c7de6b487aef6d7d64) )
	ROM_LOAD( "ld_13.bin",    0x060000, 0x10000, CRC(a1a598ac) SHA1(a0d24d9125cd502b57adf9167cb61e8864d521ce) )
	ROM_LOAD( "ld_14.bin",    0x070000, 0x10000, CRC(edf515cc) SHA1(8dc68d1d4e480afe9614ea85e2eced3fd3917484) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_01.bin",    0x000000, 0x08000, CRC(ad3c6f87) SHA1(1a5ef003c0eb641484921dc0c11450c53ee315f5) ) /* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_17.bin",    0x000000, 0x10000, CRC(7188bfdd) SHA1(26c47af6abb4e6f5e11e2dd6b56113a54c0e6269) ) /* tiles */
	ROM_LOAD( "ld_18.bin",    0x010000, 0x10000, CRC(a62af66a) SHA1(240dafcb03011cf51bfe9d01bec4aceac64d5760) )
	ROM_LOAD( "ld_19.bin",    0x020000, 0x10000, CRC(4b762e50) SHA1(95b3413f67d2e9ebea2a8331945a572a3d824cc1) )
	ROM_LOAD( "ld_20.bin",    0x030000, 0x10000, CRC(b140188e) SHA1(491af082789a11c809c2798da6ae5e52a2b1d986) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_28.bin",    0x000000, 0x10000, CRC(06778248) SHA1(09663db5f07961a432feb4f82847a2f9741b34ad) ) /* tiles */
	ROM_LOAD( "ld_26.bin",    0x010000, 0x10000, CRC(b0edac81) SHA1(2ba1f864b7f8047b20206063d4e9956ef1d1ad34) )
	ROM_LOAD( "ld_24.bin",    0x020000, 0x10000, CRC(66eac4df) SHA1(b2604f6fd443071deb2729f4381e6fe3a2069a33) )
	ROM_LOAD( "ld_22.bin",    0x030000, 0x10000, CRC(f80f8812) SHA1(2483b272b51ab15c47eb0b48df68b7c3b05d4d35) )
	ROM_LOAD( "ld_27.bin",    0x040000, 0x10000, CRC(48c78675) SHA1(27b03cd1a5335b60953e5dc4888264598e63c147) )
	ROM_LOAD( "ld_25.bin",    0x050000, 0x10000, CRC(c541ae9a) SHA1(b1d6acab76cba77ea6b9fe6fc770b6a6d6960a77) )
	ROM_LOAD( "ld_23.bin",    0x060000, 0x10000, CRC(d817332c) SHA1(c1c3d70a42eb01237bcbe8e274f7022e74c8c715) )
	ROM_LOAD( "ld_21.bin",    0x070000, 0x10000, CRC(b74f0c0e) SHA1(866e3c65fd5dd7099423baefd09eb2b7da7e8392) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.3d",    0x0000, 0x0100, CRC(729a1ddc) SHA1(eb1d48785a0f187a4cb9c164e6c82481268b3174) )	/* priority (not used) */
ROM_END

ROM_START( lstduela )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "06",   0x00000, 0x20000, CRC(0e71acaf) SHA1(e804c77bfd768ae2fc1917bcec1fd0ec7418b780) )
	ROM_LOAD16_BYTE( "05",   0x00001, 0x20000, CRC(47a85bea) SHA1(9d6b2a4e27c84ffce8ed58aa1b314c67c7314932) )
	ROM_LOAD16_BYTE( "04",   0x40000, 0x10000, CRC(aa4bf001) SHA1(3f14b174016c6fa4c82011d3d0f1c957096d6d93) )
	ROM_LOAD16_BYTE( "03",   0x40001, 0x10000, CRC(bbaac8ab) SHA1(3c5773e39e7a96ef62da7b846ce4099222b3e66b) )

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "ld_02.bin",    0x0000, 0x10000, CRC(91834d0c) SHA1(aaa63b8470fc19b82c25028ab27675a7837ab9a1) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_09.bin",    0x000000, 0x10000, CRC(f8fd5243) SHA1(fad80d8959f50a83eb2e47788a8183284d19bea6) ) /* sprites */
	ROM_LOAD( "ld_10.bin",    0x010000, 0x10000, CRC(b49ad746) SHA1(4e609982d60155b0df13a156c37bdf2a25626632) )
	ROM_LOAD( "ld_11.bin",    0x020000, 0x10000, CRC(1a0d180e) SHA1(a68a7f5d00da99a8068876fd2d61c726047aca80) )
	ROM_LOAD( "ld_12.bin",    0x030000, 0x10000, CRC(b2745e26) SHA1(b511631fe4e21f3d2dc7440b3f69cd5edb43d20e) )
	ROM_LOAD( "ld_15.bin",    0x040000, 0x10000, CRC(96b13bbc) SHA1(f2df8d4f11e9192063063ff2e9e4fe76971c5b24) )
	ROM_LOAD( "ld_16.bin",    0x050000, 0x10000, CRC(9d80f7e6) SHA1(ce7c10eba6a9f6a1fad655c7de6b487aef6d7d64) )
	ROM_LOAD( "ld_13.bin",    0x060000, 0x10000, CRC(a1a598ac) SHA1(a0d24d9125cd502b57adf9167cb61e8864d521ce) )
	ROM_LOAD( "ld_14.bin",    0x070000, 0x10000, CRC(edf515cc) SHA1(8dc68d1d4e480afe9614ea85e2eced3fd3917484) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_01.bin",    0x000000, 0x08000, CRC(ad3c6f87) SHA1(1a5ef003c0eb641484921dc0c11450c53ee315f5) ) /* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_17.bin",    0x000000, 0x10000, CRC(7188bfdd) SHA1(26c47af6abb4e6f5e11e2dd6b56113a54c0e6269) ) /* tiles */
	ROM_LOAD( "ld_18.bin",    0x010000, 0x10000, CRC(a62af66a) SHA1(240dafcb03011cf51bfe9d01bec4aceac64d5760) )
	ROM_LOAD( "ld_19.bin",    0x020000, 0x10000, CRC(4b762e50) SHA1(95b3413f67d2e9ebea2a8331945a572a3d824cc1) )
	ROM_LOAD( "ld_20.bin",    0x030000, 0x10000, CRC(b140188e) SHA1(491af082789a11c809c2798da6ae5e52a2b1d986) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_28.bin",    0x000000, 0x10000, CRC(06778248) SHA1(09663db5f07961a432feb4f82847a2f9741b34ad) ) /* tiles */
	ROM_LOAD( "ld_26.bin",    0x010000, 0x10000, CRC(b0edac81) SHA1(2ba1f864b7f8047b20206063d4e9956ef1d1ad34) )
	ROM_LOAD( "ld_24.bin",    0x020000, 0x10000, CRC(66eac4df) SHA1(b2604f6fd443071deb2729f4381e6fe3a2069a33) )
	ROM_LOAD( "ld_22.bin",    0x030000, 0x10000, CRC(f80f8812) SHA1(2483b272b51ab15c47eb0b48df68b7c3b05d4d35) )
	ROM_LOAD( "ld_27.bin",    0x040000, 0x10000, CRC(48c78675) SHA1(27b03cd1a5335b60953e5dc4888264598e63c147) )
	ROM_LOAD( "ld_25.bin",    0x050000, 0x10000, CRC(c541ae9a) SHA1(b1d6acab76cba77ea6b9fe6fc770b6a6d6960a77) )
	ROM_LOAD( "ld_23.bin",    0x060000, 0x10000, CRC(d817332c) SHA1(c1c3d70a42eb01237bcbe8e274f7022e74c8c715) )
	ROM_LOAD( "ld_21.bin",    0x070000, 0x10000, CRC(b74f0c0e) SHA1(866e3c65fd5dd7099423baefd09eb2b7da7e8392) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.3d",    0x0000, 0x0100, CRC(729a1ddc) SHA1(eb1d48785a0f187a4cb9c164e6c82481268b3174) )	/* priority (not used) */
ROM_END

ROM_START( lstduelb )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "ld_08.bin",    0x00000, 0x10000, CRC(43811a96) SHA1(79db50c941d8845f1642f2257c610768172923a3) )
	ROM_LOAD16_BYTE( "ld_07.bin",    0x00001, 0x10000, CRC(63c30946) SHA1(cab7374839a68483b3f94821144546cc3eb1528e) )
	ROM_LOAD16_BYTE( "ld_04.bin",    0x20000, 0x10000, CRC(46a4e0f8) SHA1(7d5fac209357090c5faeee3834c19f1d8125aac5) )
	ROM_LOAD16_BYTE( "ld_03.bin",    0x20001, 0x10000, CRC(8d5f204a) SHA1(0415b8a836a62aee1f430bc124996cb8c12ed5cf) )
	ROM_LOAD16_BYTE( "ldu-04.rom",   0x40000, 0x10000, CRC(429fb964) SHA1(78769b05e62c190d846dd08214427d1abbbe2bba) )
	ROM_LOAD16_BYTE( "ldu-03.rom",   0x40001, 0x10000, CRC(5aa4df72) SHA1(9e7315b793f09c8b422bad1ce776588e3a48d80c) )

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "ld_02.bin",    0x0000, 0x10000, CRC(91834d0c) SHA1(aaa63b8470fc19b82c25028ab27675a7837ab9a1) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_09.bin",    0x000000, 0x10000, CRC(f8fd5243) SHA1(fad80d8959f50a83eb2e47788a8183284d19bea6) ) /* sprites */
	ROM_LOAD( "ld_10.bin",    0x010000, 0x10000, CRC(b49ad746) SHA1(4e609982d60155b0df13a156c37bdf2a25626632) )
	ROM_LOAD( "ld_11.bin",    0x020000, 0x10000, CRC(1a0d180e) SHA1(a68a7f5d00da99a8068876fd2d61c726047aca80) )
	ROM_LOAD( "ld_12.bin",    0x030000, 0x10000, CRC(b2745e26) SHA1(b511631fe4e21f3d2dc7440b3f69cd5edb43d20e) )
	ROM_LOAD( "ld_15.bin",    0x040000, 0x10000, CRC(96b13bbc) SHA1(f2df8d4f11e9192063063ff2e9e4fe76971c5b24) )
	ROM_LOAD( "ld_16.bin",    0x050000, 0x10000, CRC(9d80f7e6) SHA1(ce7c10eba6a9f6a1fad655c7de6b487aef6d7d64) )
	ROM_LOAD( "ld_13.bin",    0x060000, 0x10000, CRC(a1a598ac) SHA1(a0d24d9125cd502b57adf9167cb61e8864d521ce) )
	ROM_LOAD( "ld_14.bin",    0x070000, 0x10000, CRC(edf515cc) SHA1(8dc68d1d4e480afe9614ea85e2eced3fd3917484) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_01.bin",    0x000000, 0x08000, CRC(ad3c6f87) SHA1(1a5ef003c0eb641484921dc0c11450c53ee315f5) ) /* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_17.bin",    0x000000, 0x10000, CRC(7188bfdd) SHA1(26c47af6abb4e6f5e11e2dd6b56113a54c0e6269) ) /* tiles */
	ROM_LOAD( "ld_18.bin",    0x010000, 0x10000, CRC(a62af66a) SHA1(240dafcb03011cf51bfe9d01bec4aceac64d5760) )
	ROM_LOAD( "ld_19.bin",    0x020000, 0x10000, CRC(4b762e50) SHA1(95b3413f67d2e9ebea2a8331945a572a3d824cc1) )
	ROM_LOAD( "ld_20.bin",    0x030000, 0x10000, CRC(b140188e) SHA1(491af082789a11c809c2798da6ae5e52a2b1d986) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ld_28.bin",    0x000000, 0x10000, CRC(06778248) SHA1(09663db5f07961a432feb4f82847a2f9741b34ad) ) /* tiles */
	ROM_LOAD( "ld_26.bin",    0x010000, 0x10000, CRC(b0edac81) SHA1(2ba1f864b7f8047b20206063d4e9956ef1d1ad34) )
	ROM_LOAD( "ld_24.bin",    0x020000, 0x10000, CRC(66eac4df) SHA1(b2604f6fd443071deb2729f4381e6fe3a2069a33) )
	ROM_LOAD( "ld_22.bin",    0x030000, 0x10000, CRC(f80f8812) SHA1(2483b272b51ab15c47eb0b48df68b7c3b05d4d35) )
	ROM_LOAD( "ld_27.bin",    0x040000, 0x10000, CRC(48c78675) SHA1(27b03cd1a5335b60953e5dc4888264598e63c147) )
	ROM_LOAD( "ld_25.bin",    0x050000, 0x10000, CRC(c541ae9a) SHA1(b1d6acab76cba77ea6b9fe6fc770b6a6d6960a77) )
	ROM_LOAD( "ld_23.bin",    0x060000, 0x10000, CRC(d817332c) SHA1(c1c3d70a42eb01237bcbe8e274f7022e74c8c715) )
	ROM_LOAD( "ld_21.bin",    0x070000, 0x10000, CRC(b74f0c0e) SHA1(866e3c65fd5dd7099423baefd09eb2b7da7e8392) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.3d",    0x0000, 0x0100, CRC(729a1ddc) SHA1(eb1d48785a0f187a4cb9c164e6c82481268b3174) )	/* priority (not used) */
ROM_END

ROM_START( madgear )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 256K for 68000 code */
	ROM_LOAD16_BYTE( "mg_04.rom",    0x00000, 0x20000, CRC(b112257d) SHA1(4acfd8ba0fe8d68ca7c9b0fde2b13ce0c9104258) )
	ROM_LOAD16_BYTE( "mg_03.rom",    0x00001, 0x20000, CRC(b2672465) SHA1(96d10046e67181160daebb2b07c867c08f8600dc) )
	ROM_LOAD16_BYTE( "mg_02.rom",    0x40000, 0x20000, CRC(9f5ebe16) SHA1(2183cb807157d48204d8d4d4b7555c9a7772ddfd) )
	ROM_LOAD16_BYTE( "mg_01.rom",    0x40001, 0x20000, CRC(1cea2af0) SHA1(9f4642ed2d21fa525e9fecaac6235a3653df3030) )

	ROM_REGION( 0x18000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "mg_05.rom",    0x00000,  0x08000, CRC(2fbfc945) SHA1(8066516dcf9261abee1edd103bdbe0cc18913ed3) )
	ROM_CONTINUE(             0x10000,  0x08000 )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mg_m11.rom",   0x000000, 0x10000, CRC(ee319a64) SHA1(ce8d65fdac3ec1009b22764807c03dd96b340660) )	/* Interleaved sprites */
	ROM_LOAD( "mg_m07.rom",   0x010000, 0x10000, CRC(e5c0b211) SHA1(dc4a92061c686a9d211a7b95aab2e41219508d67) )
	ROM_LOAD( "mg_m12.rom",   0x020000, 0x10000, CRC(887ef120) SHA1(9d57b497334d64df9a4ab7f15824dcc6a333f73d) )
	ROM_LOAD( "mg_m08.rom",   0x030000, 0x10000, CRC(59709aa3) SHA1(384641da58c8b5198ad4fa51cd5fd9a628bcb888) )
	ROM_LOAD( "mg_m13.rom",   0x040000, 0x10000, CRC(eae07db4) SHA1(59c4ff48d906b2bb101fbebe06383940fdff064f) )
	ROM_LOAD( "mg_m09.rom",   0x050000, 0x10000, CRC(40ee83eb) SHA1(35e11fcb3b75ada99df23715ecb955bd40e10da8) )
	ROM_LOAD( "mg_m14.rom",   0x060000, 0x10000, CRC(21e5424c) SHA1(2f7c5d974c847bb14eaf278545bca653919110ba) )
	ROM_LOAD( "mg_m10.rom",   0x070000, 0x10000, CRC(b64afb54) SHA1(5fdd4f67e6b7440448adf395b61c79b79b4f86e7) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "mg_06.rom",    0x000000, 0x08000, CRC(382ee59b) SHA1(a1da439f0585f5cafe2fb7024f1ae0527e34cd92) )	/* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-12",        0x000000, 0x40000, CRC(6c1b2c6c) SHA1(18f22129f13c6bfa7e285f0e09a35644272f6ecb) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-11",        0x000000, 0x80000, CRC(6bf81c64) SHA1(2289978c6bdb6e4f86e7094e861df147e757e249) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM */
	ROM_LOAD( "ls-06",        0x00000, 0x20000, CRC(88d39a5b) SHA1(8fb2d1d26e2ffb93dfc9cf8f23bb81eb64496c2b) )
	ROM_LOAD( "ls-05",        0x20000, 0x20000, CRC(b06e03b5) SHA1(7d17e5cfb57866c60146bea1a4535e961c73327c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.14k",   0x0000, 0x0100, CRC(7f862e1e) SHA1(7134c4f741463007a177d55922e1284d132f60e3) )	/* priority (not used) */
ROM_END

ROM_START( madgearj )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 256K for 68000 code */
	ROM_LOAD16_BYTE( "mdj_04.rom",   0x00000, 0x20000, CRC(9ebbebb1) SHA1(84a2b146c10c1635b11c3af0242fd4680994eb5a) )
	ROM_LOAD16_BYTE( "mdj_03.rom",   0x00001, 0x20000, CRC(a5579c2d) SHA1(789dcb1cdf5cae20ab497c75460ad98c33d1a046) )
	ROM_LOAD16_BYTE( "mg_02.rom",    0x40000, 0x20000, CRC(9f5ebe16) SHA1(2183cb807157d48204d8d4d4b7555c9a7772ddfd) )
	ROM_LOAD16_BYTE( "mg_01.rom",    0x40001, 0x20000, CRC(1cea2af0) SHA1(9f4642ed2d21fa525e9fecaac6235a3653df3030) )

	ROM_REGION(  0x18000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "mg_05.rom",    0x00000,  0x08000, CRC(2fbfc945) SHA1(8066516dcf9261abee1edd103bdbe0cc18913ed3) )
	ROM_CONTINUE(             0x10000,  0x08000 )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mg_m11.rom",   0x000000, 0x10000, CRC(ee319a64) SHA1(ce8d65fdac3ec1009b22764807c03dd96b340660) )	/* Interleaved sprites */
	ROM_LOAD( "mg_m07.rom",   0x010000, 0x10000, CRC(e5c0b211) SHA1(dc4a92061c686a9d211a7b95aab2e41219508d67) )
	ROM_LOAD( "mg_m12.rom",   0x020000, 0x10000, CRC(887ef120) SHA1(9d57b497334d64df9a4ab7f15824dcc6a333f73d) )
	ROM_LOAD( "mg_m08.rom",   0x030000, 0x10000, CRC(59709aa3) SHA1(384641da58c8b5198ad4fa51cd5fd9a628bcb888) )
	ROM_LOAD( "mg_m13.rom",   0x040000, 0x10000, CRC(eae07db4) SHA1(59c4ff48d906b2bb101fbebe06383940fdff064f) )
	ROM_LOAD( "mg_m09.rom",   0x050000, 0x10000, CRC(40ee83eb) SHA1(35e11fcb3b75ada99df23715ecb955bd40e10da8) )
	ROM_LOAD( "mg_m14.rom",   0x060000, 0x10000, CRC(21e5424c) SHA1(2f7c5d974c847bb14eaf278545bca653919110ba) )
	ROM_LOAD( "mg_m10.rom",   0x070000, 0x10000, CRC(b64afb54) SHA1(5fdd4f67e6b7440448adf395b61c79b79b4f86e7) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "mg_06.rom",    0x000000, 0x08000, CRC(382ee59b) SHA1(a1da439f0585f5cafe2fb7024f1ae0527e34cd92) )	/* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-12",        0x000000, 0x40000, CRC(6c1b2c6c) SHA1(18f22129f13c6bfa7e285f0e09a35644272f6ecb) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-11",        0x000000, 0x80000, CRC(6bf81c64) SHA1(2289978c6bdb6e4f86e7094e861df147e757e249) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM */
	ROM_LOAD( "ls-06",        0x00000, 0x20000, CRC(88d39a5b) SHA1(8fb2d1d26e2ffb93dfc9cf8f23bb81eb64496c2b) )
	ROM_LOAD( "ls-05",        0x20000, 0x20000, CRC(b06e03b5) SHA1(7d17e5cfb57866c60146bea1a4535e961c73327c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.14k",   0x0000, 0x0100, CRC(7f862e1e) SHA1(7134c4f741463007a177d55922e1284d132f60e3) )	/* priority (not used) */
ROM_END

ROM_START( ledstorm )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 256K for 68000 code */
	ROM_LOAD16_BYTE( "mdu.04",    0x00000, 0x20000, CRC(7f7f8329) SHA1(9b7ecb7f5cc3f2c80e05da3b9055e2fbd64bf0ce) )
	ROM_LOAD16_BYTE( "mdu.03",    0x00001, 0x20000, CRC(11fa542f) SHA1(1cedfc471058e0d0502a1eeafcab479dca4fea41) )
	ROM_LOAD16_BYTE( "mg_02.rom", 0x40000, 0x20000, CRC(9f5ebe16) SHA1(2183cb807157d48204d8d4d4b7555c9a7772ddfd) )
	ROM_LOAD16_BYTE( "mg_01.rom", 0x40001, 0x20000, CRC(1cea2af0) SHA1(9f4642ed2d21fa525e9fecaac6235a3653df3030) )

	ROM_REGION(  0x18000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "mg_05.rom",    0x00000,  0x08000, CRC(2fbfc945) SHA1(8066516dcf9261abee1edd103bdbe0cc18913ed3) )
	ROM_CONTINUE(             0x10000,  0x08000 )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "mg_m11.rom",   0x000000, 0x10000, CRC(ee319a64) SHA1(ce8d65fdac3ec1009b22764807c03dd96b340660) )	/* Interleaved sprites */
	ROM_LOAD( "07",           0x010000, 0x10000, CRC(7152b212) SHA1(b021496e8b3c22c018907e6e374a7401d3843570) )
	ROM_LOAD( "mg_m12.rom",   0x020000, 0x10000, CRC(887ef120) SHA1(9d57b497334d64df9a4ab7f15824dcc6a333f73d) )
	ROM_LOAD( "08",           0x030000, 0x10000, CRC(72e5d525) SHA1(209def4206e9b66be9879f0105d3f04980f156da) )
	ROM_LOAD( "mg_m13.rom",   0x040000, 0x10000, CRC(eae07db4) SHA1(59c4ff48d906b2bb101fbebe06383940fdff064f) )
	ROM_LOAD( "09",           0x050000, 0x10000, CRC(7b5175cb) SHA1(8d8d4953dd787308bed75345af6789899d2afded) )
	ROM_LOAD( "mg_m14.rom",   0x060000, 0x10000, CRC(21e5424c) SHA1(2f7c5d974c847bb14eaf278545bca653919110ba) )
	ROM_LOAD( "10",           0x070000, 0x10000, CRC(6db7ca64) SHA1(389cc93b9bfe2824a0de9796e79c6d452d09567e) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "06",           0x000000, 0x08000, CRC(54bfdc02) SHA1(480ef755425aed9e0149bdb90bf30ddaef2be192) )	/* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-12",        0x000000, 0x40000, CRC(6c1b2c6c) SHA1(18f22129f13c6bfa7e285f0e09a35644272f6ecb) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-11",        0x000000, 0x80000, CRC(6bf81c64) SHA1(2289978c6bdb6e4f86e7094e861df147e757e249) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM */
	ROM_LOAD( "ls-06",        0x00000, 0x20000, CRC(88d39a5b) SHA1(8fb2d1d26e2ffb93dfc9cf8f23bb81eb64496c2b) )
	ROM_LOAD( "ls-05",        0x20000, 0x20000, CRC(b06e03b5) SHA1(7d17e5cfb57866c60146bea1a4535e961c73327c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.14k",   0x0000, 0x0100, CRC(7f862e1e) SHA1(7134c4f741463007a177d55922e1284d132f60e3) )	/* priority (not used) */
ROM_END

ROM_START( ledstrm2 )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 256K for 68000 code */
	ROM_LOAD16_BYTE( "lsu-04.bin", 0x00000, 0x20000, CRC(56a2f079) SHA1(da581c117d92ac5c1e8e44324f1aed2858a3cdc8) )
	ROM_LOAD16_BYTE( "lsu-03.bin", 0x00001, 0x20000, CRC(9b6408c0) SHA1(8ef8349f58c62a2d626b1053eae2032d168d602c) )
	ROM_LOAD16_BYTE( "ls-02.bin", 0x40000, 0x20000, CRC(05c0285e) SHA1(b155d2d0c41f614bd324813c5d3d87a6765ad812) )
	ROM_LOAD16_BYTE( "ls-01.bin", 0x40001, 0x20000, CRC(8bf934dd) SHA1(f2287a4361af4986eb010dfbfb6de3a3d4124937) )

	ROM_REGION(  0x18000 , "audiocpu", 0 ) /* audio CPU */
	ROM_LOAD( "ls-07.bin",    0x00000,  0x08000, CRC(98af7838) SHA1(a0b87b9ce3c1b0e5d7696ffaab9cea483b9ee928) )
	ROM_CONTINUE(             0x10000,  0x08000 )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_DISPOSE ) // Sprite roms not dumped from this revision of the game
	ROM_LOAD( "mg_m11.rom",   0x000000, 0x10000, BAD_DUMP CRC(ee319a64) SHA1(ce8d65fdac3ec1009b22764807c03dd96b340660) )	/* Interleaved sprites */
	ROM_LOAD( "07",           0x010000, 0x10000, BAD_DUMP CRC(7152b212) SHA1(b021496e8b3c22c018907e6e374a7401d3843570) )
	ROM_LOAD( "mg_m12.rom",   0x020000, 0x10000, BAD_DUMP CRC(887ef120) SHA1(9d57b497334d64df9a4ab7f15824dcc6a333f73d) )
	ROM_LOAD( "08",           0x030000, 0x10000, BAD_DUMP CRC(72e5d525) SHA1(209def4206e9b66be9879f0105d3f04980f156da) )
	ROM_LOAD( "mg_m13.rom",   0x040000, 0x10000, BAD_DUMP CRC(eae07db4) SHA1(59c4ff48d906b2bb101fbebe06383940fdff064f) )
	ROM_LOAD( "09",           0x050000, 0x10000, BAD_DUMP CRC(7b5175cb) SHA1(8d8d4953dd787308bed75345af6789899d2afded) )
	ROM_LOAD( "mg_m14.rom",   0x060000, 0x10000, BAD_DUMP CRC(21e5424c) SHA1(2f7c5d974c847bb14eaf278545bca653919110ba) )
	ROM_LOAD( "10",           0x070000, 0x10000, BAD_DUMP CRC(6db7ca64) SHA1(389cc93b9bfe2824a0de9796e79c6d452d09567e) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-08.bin",    0x000000, 0x08000, CRC(8803cf49) SHA1(7a01a05f760d8e2472fdbc1d10b53094babe295e) )	/* 8x8 text */

	ROM_REGION( 0x40000, "gfx3", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-12",        0x000000, 0x40000, BAD_DUMP CRC(6c1b2c6c) SHA1(18f22129f13c6bfa7e285f0e09a35644272f6ecb) )

	ROM_REGION( 0x80000, "gfx4", ROMREGION_DISPOSE )
	ROM_LOAD( "ls-11",        0x000000, 0x80000, BAD_DUMP CRC(6bf81c64) SHA1(2289978c6bdb6e4f86e7094e861df147e757e249) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM */
	ROM_LOAD( "ls-06",        0x00000, 0x20000, CRC(88d39a5b) SHA1(8fb2d1d26e2ffb93dfc9cf8f23bb81eb64496c2b) )
	ROM_LOAD( "ls-05",        0x20000, 0x20000, CRC(b06e03b5) SHA1(7d17e5cfb57866c60146bea1a4535e961c73327c) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.14k",   0x0000, 0x0100, CRC(7f862e1e) SHA1(7134c4f741463007a177d55922e1284d132f60e3) )	/* priority (not used) */
ROM_END

/******************************************************************************/

GAME( 1988, lastduel, 0,        lastduel, lastduel, 0, ROT270, "Capcom", "Last Duel (US set 1)", 0 )
GAME( 1988, lstduela, lastduel, lastduel, lastduel, 0, ROT270, "Capcom", "Last Duel (US set 2)", 0 )
GAME( 1988, lstduelb, lastduel, lastduel, lastduel, 0, ROT270, "bootleg", "Last Duel (bootleg)", 0 )
GAME( 1989, madgear,  0,        madgear,  madgear,  0, ROT270, "Capcom", "Mad Gear (US)", 0 )
GAME( 1989, madgearj, madgear,  madgear,  madgear,  0, ROT270, "Capcom", "Mad Gear (Japan)", 0 )
GAME( 1988, ledstorm, madgear,  madgear,  madgear,  0, ROT270, "Capcom", "Led Storm (US)", 0 )
GAME( 1988, ledstrm2, madgear,  madgear,  madgear,  0, ROT270, "Capcom", "Led Storm Rally 2011 (US)", 0 )
