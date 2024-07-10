

#include <math.h>
#include "DSP28x_Project.h"                         // Arquivo de cabecalho do dispositivo e exemplos incluem arquivo
#include <stdio.h>

#define PI 3.14159265358979323846

                                                    //
__interrupt void funcao_transformada(void);                 // declaracao da funcao de interrupcao
void Adc_Config(void);                              // declaracao de funcao
void freq (float);                                  //
void dutty (float);                                 //
                                                    //
//float MatrizCP   [3][3] = {1/2 , 1/2 , 1/2 , cos(rho) , -1/2*cos(rho)+(sqrt(3)/2)*sen(rho) , -1/2*cos(rho)-(sqrt(3)/2)*sen(rho) , -sen(rho) , 1/2*cos(rho)+(sqrt(3)/2)*sen(rho) , 1/2*cos(rho)-(sqrt(3)/2)*sen(rho)};
//float MatrizABC  [3][1] = {cos(w*t + thetazero) , cos(w*t + thetazero -2*PI/3 ) , cos(w*t + thetazero -4*PI/3 )};
//float acumulador [3][1] = {0 ,0 ,0};

float w = 1;
float t = 0;
float thetazero = 0;
float rho = 0;

float I0 = 0;
float Id = 0;
float Iq = 0;

float idfa = 1;

//Alteração


int i = 0;
int j = 0;
int aux = 0;

int contador = 0;


int main(void)
{






                                                    //
    InitSysCtrl();                                  //  inicializa o sistema (clk, ...)
    InitEPwm1Gpio();                                //  inicializa o PWM
    DINT;                                           //  desabilita interrupcoes
    InitPieCtrl();                                  //  inicializa o tabela de interrupcoes (acessa pievect)
    IER = 0x0000;                                   //  desabilita interrupcoes da CPU
    IFR = 0x0000;                                   //  limpa as flags de interrupcao
    InitPieVectTable();                             //  trabalha junto com initpiectrl  (configura priorirdades de interrupcoes)
                                                    //
    EALLOW;                                         //
    PieVectTable.ADCINT1 = &funcao_transformada;                // interrupcao associada ao endere�o da fun��o adc_isr
    EDIS;                                           //
                                                    //
    InitAdc();                                      // inicializacao o ADC
    AdcOffsetSelfCal();                             // ??????????????????????
                                                    //
                                                    // Habilita ADCINT1 no PIE
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;              // Habilita INT 1.1 no PIE
    IER |= M_INT1;                                  // Habilita interrupcao de CPU 1
    EINT;                                           // Habilita interrupcao global INTM
    ERTM;                                           // Habilita DBGM de interrupcao global em tempo real
                                                    //
                                                    //
    EALLOW;                                         //
    EPwm1Regs.TBPRD = 3000;                         // 30K Hz                                                                           //  ciclos de clock ate o pico da onda portadora , freq = freq_clk / n_ciclos (freq_clk = 90MHz)
    EPwm1Regs.CMPA.half.CMPA = 1500;                // Duty cycle de 50% (3000/2)                                                       //  configura o valor de comparacao com canal A
    EPwm1Regs.CMPB = 0;                             // N�o utilizado neste exemplo                                                      //  configura o valor de comparacao com canal B
    EPwm1Regs.TBCTR = 0;                            //                                                                                  //  incrementa ate alcan�ar o TBPRD
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;      // Modo de contagem: Up-Down mode                                                   //  contagem ascendente, descendente ou ascendente-descendente
    EPwm1Regs.TBCTL.bit.PHSEN = 0;                  //                                                                                  //  sincroniza��o de fase, permitindo que o sinal PWM seja sincronizado com outro sinal
    EPwm1Regs.TBCTL.bit.PRDLD = 1;                  // Carregar o valor do per�odo imediatamente // fazer apresenta��o para explica��o  //  Determina quando o valor do per�odo (PRD) deve ser carregado. Se for 1, o carregamento � imediato
    EPwm1Regs.TBCTL.bit.SYNCOSEL = 0;               // Sem sincroniza��o                                                                //  habilita ou desabilita a sincroniza��o do PHSEN
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;        // Divisor de clock em alta velocidade       // fazer apresenta��o para explica��o  //  Define o divisor de clock para o timer do PWM em alta velocidade
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;                 // Divisor de clock                          // pesquisar dps                       //  Define o divisor de clock para o timer do PWM
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;     // Modo de sombra: Atualiza��o direta                                               //  guarda a informa��o do CMPA para comparar com TBCTR
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;     // Modo de sombra: Atualiza��o direta                                               //  Configura o modo de sombra para o registrador CMPB
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;   // Carregar CMPA imediatamente                                                      //  Configura quando o valor de compara��o para o canal A � carregado. imediato ou no pr�ximo ciclo
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;   // N�o utilizado neste exemplo                                                      //  Configura quando o valor de compara��o para o canal B � carregado
    EPwm1Regs.AQCTLA.bit.ZRO = AQ_SET;              // Set no evento de contagem zerada                                                 //  Configura a a��o que ocorre quando o contador do timer atinge zero para o canal A
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;            // Clear no evento de contagem igual a CMPA                                         //  Configura a a��o que ocorre quando o contador do timer atinge o valor de compara��o para o canal A
    EPwm1Regs.AQCTLB.bit.ZRO = AQ_SET;              //                                                                                  //  Configura a a��o que ocorre quando o contador do timer atinge zero para o canal B
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;            //                                                                                  //  Configura a a��o que ocorre quando o contador do timer atinge o valor de compara��o para o canal B
    EPwm1Regs.ETSEL.bit.SOCAEN   = 1;               // Habilitar SOC em um grupo
    EPwm1Regs.ETSEL.bit.SOCASEL  = 4;               // Selecione SOC do CMPA na contagem crescente
    EPwm1Regs.ETPS.bit.SOCAPRD   = 1;               // Gerar pulso no 1º evento
    EDIS;                                           //
                                                    //
    EALLOW;                                         //
        SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;      // inicia todos os timers sincronizados
    EDIS;                                           //
                                                    //
                                                    //
    EALLOW;                                         //
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;          // Ativar modo de sobreposicao                              (1 sem sobreposicao , 0 com sobreposicao)
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1;            // interrupcao gerada pelo ADC                              (-- no incio da conversao , -- no final da conversao) ??????????????????????
    AdcRegs.INTSEL1N2.bit.INT1E     = 1;            // interrupcao do ADCINT1                                   (1 habilita interrupcao , 0 desabilita interrupcao)
    AdcRegs.INTSEL1N2.bit.INT1CONT  = 0;            // interrupcao do ADCINT1 continua ou uma vez               (1 continuo , 0 uma vez)
    AdcRegs.INTSEL1N2.bit.INT1SEL   = 1;            // qual conversor inicia a interrupcao do ADCINT1           (1 EOC1 , 0 --) ??????????????????????
    AdcRegs.ADCSOC0CTL.bit.CHSEL    = 4;            // canal SOC0 analogico sera convertido para qual ADCIN     (4 ADCINA4)
    AdcRegs.ADCSOC1CTL.bit.CHSEL    = 2;            // canal SOC1 analogico sera convertido para qual ADCIN     (2 ADCINA2)
    AdcRegs.ADCSOC0CTL.bit.TRIGSEL  = 5;            // fonte de disparo(trigger) do SOC0 no EPWM1A              (devido ao round-robin, o SOC0 converte primeiro e depois o SOC1)
    AdcRegs.ADCSOC1CTL.bit.TRIGSEL  = 5;            // fonte de disparo(trigger) do SOC1 no EPWM1A              (devido ao round-robin, o SOC0 converte primeiro e depois o SOC1)
    AdcRegs.ADCSOC0CTL.bit.ACQPS    = 6;            // define a janela SOC0 S/H para 7 ciclos de relogio ADC    (6 ACQPS mais 1)
    AdcRegs.ADCSOC1CTL.bit.ACQPS    = 6;            // define a janela SOC1 S/H para 7 ciclos de relogio ADC    (6 ACQPS mais 1)
    EDIS;


    while(1)
    {
    }


}



__interrupt void  funcao_transformada(void)                             // fun��o usada apos a interrupcao do ADC
{


     if(contador>=30000)
             {
                 contador = 0;
                 t++;
             }

     rho = 2*PI* contador/30000;

     I0 =  1/2     *cos(w*t + thetazero)  +  1/2                              *cos(w*t + thetazero -2*PI/3 )  +  1/2*cos(w*t + thetazero -4*PI/3 );
     Id =  cos(rho)*cos(w*t + thetazero)  -  1/2*cos(rho)+(sqrt(3)/2)*sin(rho)*cos(w*t + thetazero -2*PI/3 )  -  1/2*cos(rho)-(sqrt(3)/2)*sin(rho)*cos(w*t + thetazero -4*PI/3 );
     Iq = - sin(rho)*cos(w*t + thetazero) +  1/2*cos(rho)+(sqrt(3)/2)*sin(rho)*cos(w*t + thetazero -2*PI/3 )  +  1/2*cos(rho)-(sqrt(3)/2)*sin(rho)*cos(w*t + thetazero -4*PI/3 );



    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;                 // Clear ADCINT1 flag reinitialize for next SOC
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;               // Acknowledge interrupt to PIE

}

/*
   __interrupt void  funcao_transformada(void)                             // fun��o usada apos a interrupcao do ADC




// incrementar rho
// incrementar t


if(contador>=30000)
        {
            contador = 0;
            t++;
        }

rho = 2*PI* contador/30000;


for(i=0;i<3;i++){
    for(j=0;j<1;j++){
        for(aux=0;aux<3;aux++){

            acumulador[i][j] = acumulador[i][j] + MatrizCP[i][aux] * MatrizABC[aux][j];

        }

    }
}

*/
