#include "analog_sw.h"

//------------------------------------
// アナログＳＷ判定定数定義
//------------------------------------
const unsigned char q_asw[16] = {   6, 20, 35, 50, 66, 81, 95,110,
                                  125,139,153,168,182,197,211,231,}

void asw_ini( void )
{
	ADCSRA = 0b10000000;    // A/D変換 制御/状態ﾚｼﾞｽﾀA
				//         |||||+++--ADPS2,1,0=000 : ADCクロック ck/(2): 変換時間｛1/(1.2e6/2)*13=0.000022｝
				//         ||||+--------ADIE A/D変換完了割り込み：>不許可
				//         |||+---------ADIF A/D変換完了割り込み要>求ﾌﾗｸﾞ A/D変換が完了で１
				//         ||+----------ADATE A/D変換自動起動 : 不>許可
				//         |+-----------ADSC １でA/D変換開始
				//         +------------ADEN A/D許可
	ADMUX = 0b00100010;     // A/D多重器選択ﾚｼﾞｽﾀ
				//        ||||||++------MUX1,MUX0  00:ADC0(PB5), 01:ADC1(PB2), 10:ADC2(PB4), 11:ADC3(PB3)
				//        |||+++-------- - 予約
				//        ||+-----------ADLAR 左詰めで格納
				//        |+------------基準電圧選択 0:VCC, 1:内部1.1[v]
				//        +------------- - 予約
}

/*--------------------------------------------------*/
/*        アナログ入力  ADC2:PB4値取得              */
/*--------------------------------------------------*/
void asw_main( void )
{
        ADCSRA |= 1<<ADIF;                      //変換>完了フラグをリセット
                                                //書き>込む事でリセットできる。変換前に呼び出す事
        ADCSRA |= 1<<ADSC;                      //ADSC(A/D変換開始)

        while( (ADCSRA & (1<<ADIF) )==0 ) ;     //変換>完了フラグが1になるまで待機
        aswin=ADCH;                             //aswinにデータの上位8ビットを格納
}

/*--------------------------------------------------*/
/*        アナログスイッチ判定                      */
/*--------------------------------------------------*/
void asw( void )
{
        unsigned char i;                        // ワークカウンタ

        zabswu = 0xff;                          // 立ち上がりフラグ未押下
        zabswd = 0xff;                          // 立ち下がりフラグ未押下

                                                // ***** ＳＷ番号検索 *****
        for( i = 0; i < sizeof( q_asw ); i++ ) { // A/D値検索
                if( aswin < q_asw[i] )  break;  // A/D＜q_asw[i] ならＳＷ番号確定
        }
        if( i == sizeof( q_asw ) ) i = 0xff;    // 範囲外（未押下）なら0xffに補正
        zabswn = i;                             // 今回ＳＷ番号＝00〜0fh,0ffH


                                                // ***** 立ち上がり／立ち下がり処理 *****
        if( zabswn == zabswz ) {                        // *** 今回＝前回 の場合 ***
                if( ++zacsw >= 21 ) {           // 連続カウンタ　＋１, ２１回以上？
                        zacsw = 20;             // 連続２１回以上は２０回に補正
                        return;
                }
                if( zacsw == 20 ) {             // １９→２０回の場合
                        if( zabswk == 0xff ) {  // 確定値が未押下なら
                                zabswd = zabswn;// 立ち下がりフラグセット
                        } else {
                                zabswu = zabswk;// 立ち上がりフラグセット
                        }
                        zabswk = zabswn;        // 確定値セット
                }
                return;
        } else {                                // *** 今回≠前回 の場合 ***
                                                // 確定値＝未押下or確定値≠今回なら
                if( ( zabswk == 0xff ) || ( zabswk != zabswn ) )
                        zacsw = 1;              // 連続押下カウンタ初期化
        }
        zabswz = zabswn;                        // 前回←今回
}


