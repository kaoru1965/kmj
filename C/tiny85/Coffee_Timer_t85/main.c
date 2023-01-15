





/* ヘッダファイル */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "sw.h"
#include "soft_timer.h"

/* グローバル変数 */
volatile unsigned char f_1ms     = 0;
volatile unsigned int  sound_ptn = 0;
volatile unsigned char sound_on  = 0;
volatile unsigned char tone_no;  /* initializeで設定 */
	 unsigned char sw_click  = 0;
	 unsigned char repeat;  /* トーン繰り返し *//* initializeで設定 */
	 unsigned char LED_ON   = 1;  /*  */
         unsigned char VOL      = 2;  /* 音の大きさ２段階 */
static unsigned char EEMEM EEPROM_V1;  /* EEPROM_V1 　を宣言 */


/* プロトタイプ宣言 */
int main( void );
void initialize( void );
ISR(TIM0_COMPA_vect);
ISR(TIM1_COMPA_vect);

void Tone0( void ); /* 0Hz    OFF                      */
void Tone1( void ); /* 396Hz　マザー(原母)             */
void Tone2( void ); /* 417Hz 　トランス(許容への酔い)  */
void Tone3( void ); /* 528Hz　セルフ(奇跡の輝き)       */
void Tone4( void ); /* 639Hz　アーティスト(高度な自己) */
void Tone5( void ); /* 741Hz　クリアリング(清め)       */
void Tone6( void ); /* 852Hz　インスパイヤ(天の智)     */
void Tone7( void ); /* 888Hz  852より天に近い          */
void Tone8( void ); /* 432Hz　リアルA(真の基音)        */
void Tone9( void ); /* 444Hz　蘇生(復活)               */
void Tone10( void );/* 153Hz　レリーフ(安寧) 忘れる    */

void sound_main( void );
void tone_change( void );


int main( void )
{

	initialize();  /* 初期化： 一般                 */
	t_ini();       /* 初期化： ソフトタイマー       */
	sw_ini();      /* 初期化： ２回照合スイッチ入力 */
	eeprom_busy_wait();
	LED_ON=eeprom_read_byte( &EEPROM_V1 );
	//eeprom_write_byte(&EEPROM_V1, LED_ON );

	sei();

	while( 1 ) {
		sw_main();
		t_main();
		sound_main();


//   電源投入時：LED_ON、圧電スピーカーOFF
//
//   １クリック：Tone0--Tone1--...--Tone10--..--Tone0... 繰り返し。。。
//
//   ２クリック：現在値が、Tone0の場合
//   　　　　　　　　Tone1--...--Tone10--..--Tone1... 繰り返し。。。
//               現在値が、Tone0以外の場合
//   　　　　　　　　現在値Toneから.順番に... 繰り返し。。。
//   
//   ３クリック：電源投入時に戻す（リセット）
//
		if( sw_dn & ( 1<<PB4 ) ) {
			sound_ptn = 0b1000000000000000;
			//if( ( ! t_tup( 0 ) ) && ( sw_click == 0 ) ) t_req( 0 );
			if( ( ! t_tup( 0 ) ) ) t_req( 0 );
			sw_click++;
		}


		if( t_tup( 0 ) ) {
			if( ( sw_click == 1 ) ) {
				repeat = 0;
				if( ++tone_no > 10 ) tone_no = 0;
				if( tone_no == 0 ) sound_ptn = 0b1010001111000000;
			} else if( ( sw_click == 2 )  ) {
				repeat = 1;
				t_req( 1 );
				sound_ptn = 0b1010101010101010;
				if( tone_no == 0 ) tone_no = 1;
			} else if( ( sw_click == 3 ) ) {
				repeat = 0;
				tone_no = 0;
				sound_ptn = 0b1111000000110000;
			} else if( ( sw_click == 5 ) ) {
				sound_ptn = 0b1000000000000001;
				if( LED_ON )
					LED_ON = 0;
				else
					LED_ON = 1;
				eeprom_busy_wait();
				eeprom_write_byte(&EEPROM_V1, LED_ON );
			}
			sw_click = 0;
			t_can( 0 );
			tone_change();
		}


		if( repeat == 1 ) {
			if( t_tup( 1 ) ) {
				t_req( 1 );
				if( ++tone_no > 10 ) tone_no = 1;
			}
		}


		if( LED_ON )  PORTB |= 1<<PB3;
		else          PORTB &= ~( 1<<PB3 );

		while( f_1ms < 5 ) ; /* 0.2 x 5 = 1[ms] 待機     */
		f_1ms = 0;            /* フラグリセット */
	}
	return 0;
}

void initialize( void )
{
//   電源投入時：LED_ON、圧電スピーカーOFF
//   Timer0:CTC動作: main() を0.2[ms] x 5 = 1[ms] で実行
//   Timer1:CTC動作: クロックを内部 PLL=64[MHz] を選択：圧電スピーカー
	TCCR0A = 1 << WGM01;
	TCCR0B = 0b011 << CS00;  /* 8000000/64 */
	OCR0A  = 8000000/64/5000-1; /* 24 */
	TCNT0=0;
	DDRB = ( 1 << PB3 ) | ( 1<<PB1 ) | ( 1 << PB0 ) ;  /* LED, 圧電スピーカー */
	asm( "nop" );
	PORTB |= 1<<PB4;   /* タクトスイッチ：プルアップ */

	/* タイマークロック：PLL == 64[MHz] へ切り替え */
	PLLCSR |= 1<<PLLE;                     /* PLL許可             */
	_delay_us( 100 );                      /*100[us]待機          */
	while( !( PLLCSR & ( 1<<PLOCK ) ) ) ;  /* PLOCK == 1 まで待機 */
	PLLCSR |= 1<<PCKE;
	asm( "nop" );
	//Tone0();
	/* タイマークロック：PLL == 64[MHz] へ切り替え: ここまで */

	TIMSK  = ( 1 << OCIE0A ) | ( 1<<OCIE1A );
	t_req( 1  ); /* トーン繰り返し、タイマー時間リクエスト */
	repeat  = 1;  /* トーン繰り返し */
	tone_no = 1;  /* 最初のトーン選択 */
}

void sound_main( void )
{
	/* 50[ms] 周期で実行 */
	static unsigned char count = 50;

	if( --count == 0 ) {
		count = 50;

		if( sound_ptn > 0 ) {
			Tone0();
			sound_on = ( sound_ptn & 0x8000 ) ? 1 : 0;
			sound_ptn <<= 1;
			PORTB ^= 1<<PB1;
		} else {
			sound_on = 0;
			tone_change();
		}

	}
}


ISR(TIM0_COMPA_vect)
{
	++f_1ms;

	if( sound_on == 1 ) {
		TCCR1 = 0;
		PORTB ^= (1<<PB3) | (1<PB1);
	} else {
		PORTB &= ~((1<<PB3) | (1<PB1));
	}
}
ISR(TIM1_COMPA_vect)
{
}


void tone_change( void )
{

	switch( tone_no ) {
		case 0:
			Tone0();
			break;
		case 1:
			Tone1();
			break;
		case 2:
			Tone2();
			break;
		case 3:
			Tone3();
			break;
		case 4:
			Tone4();
			break;
		case 5:
			Tone5();
			break;
		case 6:
			Tone6();
			break;
		case 7:
			Tone7();
			break;
		case 8:
			Tone8();
			break;
		case 9:
			Tone9();
			break;
		case 10:
			Tone10();
			break;
		default:
			break;
	};
}


/* トーン関数 */

#define DIV_OFF    0b0000
#define DIV_2      0b0001
#define DIV_4      0b0011
#define DIV_8      0b0100
#define DIV_16     0b0101
#define DIV_32     0b0110
#define DIV_64     0b0111
#define DIV_128    0b1000
#define DIV_256    0b1001
#define DIV_512    0b1010
#define DIV_1024   0b1011
#define DIV_2048   0b1100
#define DIV_4096   0b1101
#define DIV_8192   0b1110
#define DIV_16384  0b1111



/*  0[Hz] OFF  */
void Tone0( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_OFF;
}

/*  396[Hz]  */
void Tone1( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_1024;
	OCR1C = 157;
	OCR1A = 1;
}

/*  417[Hz]  */
void Tone2( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_2048;
	OCR1C = 74;
	OCR1A = 1;
}

/*  528[Hz]  */
void Tone3( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_512;
	OCR1C = 236;
	OCR1A = 1;
}

/*  639[Hz]  */
void Tone4( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_512;
	OCR1C = 195;
	OCR1A = 1;
}

/*  741[Hz]  */
void Tone5( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_512;
	OCR1C = 168;
	OCR1A = 1;
}

/*  852[Hz]  */
void Tone6( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_512;
	OCR1C = 146;
	OCR1A = 1;
}

/*  888[Hz]  */
void Tone7( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_512;
	OCR1C = 140;
	OCR1A = 1;
}

/*  432[Hz]  */
void Tone8( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_1024;
	OCR1C = 144;
	OCR1A = 1;
}

/*  444[Hz]  */
void Tone9( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_1024;
	OCR1C = 140;
	OCR1A = 1;
}

/*  153[Hz]  */
void Tone10( void ) {
	TCCR1 = ( 1<<CTC1 ) | ( 1<<PWM1A ) | ( 3<<COM1A0 ) | DIV_2048;
	OCR1C = 203;
	OCR1A = 1;
}


