/*
 * clock_setting.c
 *
 * Created: 2021/12/30 10:58:37
 * Author : kiyot
 */ 
#include <stdint.h>
#include <stddef.h>
#include <samd51_clock.h>

#include "internal/clock_setting.h"

void initialize_clock_tree(void)
{
	//J 1MHzのクロックソースを作る
	samd51_gclk_configure_generator(CLK_GEN_NUMBER_1MHZ, SAMD51_GCLK_SRC_DFLL, 48, 0, SAMD51_GCLK_DIV_NORMAL);

	//J 32kHz backup clock
	samd51_gclk_configure_generator(CLK_GEN_NUMBER_32kHZ, SAMD51_GCLK_SRC_OSCULP32K, 0, 0, SAMD51_GCLK_DIV_NORMAL);

	//J 48MHzのクロック源をMAINとは別に用意
	samd51_gclk_configure_generator(CLK_GEN_NUMBER_48MHZ, SAMD51_GCLK_SRC_DFLL, 0, 0, SAMD51_GCLK_DIV_NORMAL);
	//J 48MHz 基準クロックをクロジェネ0番から移動させる
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_DFLL48, CLK_GEN_NUMBER_48MHZ);

	//J DFLL用のクロックを準備
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_FDPLL_32K, CLK_GEN_NUMBER_32kHZ);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_OSCCTRL_FDPLL0, CLK_GEN_NUMBER_1MHZ);

	//J DFLLを120MHzで起動
	samd51_mclk_enable(SAMD51_APBA_OSCCTRL, 1);
	SAMD51_OSC_OPT osc_opt;
	{
		osc_opt.dco_filter = 0;
		osc_opt.filter = 0;
		osc_opt.filter_en = 1;
	}
	samd51_oscillator_dpll_enable(0, SAMD51_OSC_REF_GCLK, 1000000, 120000000, &osc_opt);

	//J 120MHzになったFDPLLをGCLK0のソースに設定する
	samd51_gclk_configure_generator(CLK_GEN_NUMBER_MAIN, SAMD51_GCLK_SRC_DPLL0, 0, 0, SAMD51_GCLK_DIV_NORMAL);

	//J USBを使うのでクロックを供給しておく
	samd51_mclk_enable(SAMD51_AHB_USB, 1);
	samd51_mclk_enable(SAMD51_APBB_USB, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_USB, CLK_GEN_NUMBER_48MHZ);

	// External Interrupt
	samd51_mclk_enable(SAMD51_APBA_EIC, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_EIC, CLK_GEN_NUMBER_48MHZ);

	// Timer / Counter
	samd51_mclk_enable(SAMD51_APBA_TCn0, 1);
	samd51_mclk_enable(SAMD51_APBA_TCn1, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_TC0_TC1, CLK_GEN_NUMBER_1MHZ);

	// I2c 0/2
	samd51_mclk_enable(SAMD51_APBA_SERCOM0, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_SERCOM0_CORE, CLK_GEN_NUMBER_48MHZ);

	samd51_mclk_enable(SAMD51_APBB_SERCOM2, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_SERCOM2_CORE, CLK_GEN_NUMBER_48MHZ);

	// ADC
	samd51_mclk_enable(SAMD51_APBD_ADCn0, 1);
	samd51_gclk_configure_peripheral_channel(SAMD51_GCLK_ADC0, CLK_GEN_NUMBER_48MHZ);

	return;
}
