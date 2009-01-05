#pragma once

#ifndef __SN76496_H__
#define __SN76496_H__

WRITE8_HANDLER( sn76496_0_w );
WRITE8_HANDLER( sn76496_1_w );
WRITE8_HANDLER( sn76496_2_w );
WRITE8_HANDLER( sn76496_3_w );
WRITE8_HANDLER( sn76496_4_w );

SND_GET_INFO( sn76496 );
SND_GET_INFO( sn76489 );
SND_GET_INFO( sn76489a );
SND_GET_INFO( sn76494 );
SND_GET_INFO( gamegear );
SND_GET_INFO( smsiii );

#endif /* __SN76496_H__ */
