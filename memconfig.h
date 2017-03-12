#pragma once

value_t framecounter = 0x02; // size 2
value_t musicflag = 0x04;
value_t executewait = 0x05; // size 2
value_t irqfuk_begin = 0xF000;

value_t if_startirq = irqfuk_begin   + 0;
value_t if_setirq1 = irqfuk_begin    + 3;
value_t if_unsetirq1 = irqfuk_begin  + 6;
value_t if_setirq2 = irqfuk_begin    + 9;
value_t if_unsetirq2 = irqfuk_begin  + 12;
value_t if_startmusic = irqfuk_begin + 15;
value_t if_executenext = irqfuk_begin + 18;
value_t if_executenext_nowait = irqfuk_begin + 21;


value_t effu_pumpum = 0xA000;

value_t sync_sprites = 0x110;

//value_t sync_lightappear = 0x40;
value_t sync_lightappear = 0x200;
value_t sync_lightchangea = 0x300;
value_t sync_lightrecedea = 0x550;

value_t sync_fade = 0x650;

value_t sync_kalloload = 0x700;
//value_t sync_kalloload = 0x10;

value_t sync_lightappear2 = 0x770;
value_t sync_sprites2 = 0x830;
value_t sync_lightrecedeb = 0xb00;

//value_t sync_eyegravity = 0x20;
value_t sync_eyegravity = 0xbd0;

value_t sync_pumpum_crazysprite = 0xc95;

//value_t sync_pumpumgraffaload = 0x100;
value_t sync_pumpumgraffaload = 0xdc0; //de0 ennen

value_t sync_laserload = 0xe50;
value_t sync_laserexecute = 0xeea;

value_t sync_laser_toinenload = 0x290;
value_t sync_laser_logo = 0x400;

value_t sync_laser_load_next = 0x500;
value_t sync_mohkoexecute = 0x600;

value_t sync_mohko_logo = 0x1aA;
value_t sync_mohko_backtogfx = 0x2df;
value_t sync_mohko_gonext = 0x370;

value_t sync_msdos2_hidelogo = 0x7e0;

value_t sync_twist_gonext = 0x2A0;

value_t sync_roto_gonext = 0x200;


//#define NO_LOADING

