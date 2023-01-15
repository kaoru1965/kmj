#include <avr/io.h>
#include "soft_timer.h"


/* ソフトタイマ４本 */


      unsigned long t_counter[ 4 ];  /* タイマカウンタ    */
const unsigned long t_initial[ 4 ] = { 500,60000*3,999,999 };

/**********************************************************/
/*            初期化                                      */
/**********************************************************/
void t_ini( void )
{
	for( unsigned char i=0; i<4; ++i ) {
		t_counter[ i ] = 0xffffffff;
	}
}

/**********************************************************/
/*            メイン                                      */
/**********************************************************/
void t_main( void )
{
	for( unsigned char i=0; i<4; ++i ) {
		if( ( t_counter[ i ] != 0xffffffff ) && 
		 ( t_counter[ i ] != 0x0000 ) )
			t_counter[ i ]--; /* デクリメント */
	}
}

/**********************************************************/
/*            リクエスト                                  */
/**********************************************************/
void t_req( unsigned char i )
{
	t_counter[ i ] = t_initial[ i ];
}

/**********************************************************/
/*            キャンセル                                  */
/**********************************************************/
void t_can( unsigned char i )
{
	t_counter[ i ] = 0xffffffff;
}

/**********************************************************/
/*            タイムアップ                                */
/**********************************************************/
unsigned char t_tup( unsigned char i )
{
	return ( t_counter[ i ] == 0 )  ? 1 : 0;
}

/**********************************************************/
/*            休止中                                      */
/**********************************************************/
unsigned char t_off( unsigned char i )
{
	return ( t_counter[ i ] == 0xffffffff )  ? 1 : 0;
}

/**********************************************************/
/*            カウントダウン中                            */
/**********************************************************/
unsigned char t_run( unsigned char i )
{
	return ( 
			( t_counter[ i ] != 0xffffffff ) 
			&& 
			( t_counter[ i ] != 0x0000 ) )  ? 1 : 0;
}

/**********************************************************/
/*             強制設定                                   */
/**********************************************************/
void t_set( unsigned char i, unsigned long cnt )
{
	t_counter[ i ] = cnt;
}

