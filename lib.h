#pragma once
#include <hashembler.h>

namespace hashembler
{

static void MOV8i(value_t outaddr, value_t inval)
{
	LDAi(inval);
	STA(outaddr);
}

static void MOV8zi(value_t outaddr, value_t inval)
{
	LDAi(inval);
	STAz(outaddr);
}

static void MOV16(value_t outaddr, value_t inaddr)
{
	LDA(inaddr + 0);
	STA(outaddr + 0);
	LDA(inaddr + 1);
	STA(outaddr + 1);
}

static void MOV16z(value_t outaddr, value_t inaddr)
{
	LDAz(inaddr + 0);
	STAz(outaddr + 0);
	LDAz(inaddr + 1);
	STAz(outaddr + 1);
}
static void MOV16i(value_t outaddr, value_t inval)
{
	LDAi(LB(inval));
	STA(outaddr + 0);
	LDAi(HB(inval));
	STA(outaddr + 1);
}

static void MOV16zi(value_t outaddr, value_t inval)
{
	LDAi(LB(inval));
	STAz(outaddr + 0);
	LDAi(HB(inval));
	STAz(outaddr + 1);
}


static void ADD16zi(value_t addr, value_t con)
{
	LDAz(addr + 0);
	CLC();
	ADCi(LB(con));
	STAz(addr + 0);
	LDAz(addr + 1);
	ADCi(HB(con));
	STAz(addr + 1);
}

static void ADD16zi(value_t outaddr, value_t addr, value_t con)
{
	LDAz(addr + 0);
	CLC();
	ADCi(LB(con));
	STAz(outaddr + 0);
	LDAz(addr + 1);
	ADCi(HB(con));
	STAz(outaddr + 1);
}


static void ADD16z(value_t addr, value_t addr2)
{
	LDAz(addr  + 0);
	CLC();
	ADCz(addr2 + 0);
	STAz(addr  + 0);

	LDAz(addr  + 1);
	ADCz(addr2 + 1);
	STAz(addr  + 1);
}

static void ADD16z(value_t outaddr, value_t addr, value_t addr2)
{
	LDAz(addr  + 0);
	CLC();
	ADCz(addr2 + 0);
	STAz(outaddr  + 0);

	LDAz(addr  + 1);
	ADCz(addr2 + 1);
	STAz(outaddr  + 1);
}


static void ADD16i(value_t addr, value_t con)
{
	LDA(addr + 0);
	CLC();
	ADCi(LB(con));
	STA(addr + 0);
	LDA(addr + 1);
	ADCi(HB(con));
	STA(addr + 1);
}

void SUB16z(value_t outaddr, value_t addr1, value_t addr2)
{
	LDA(addr1 + 0);
	SEC();
	SBC(addr2 + 0);
	STA(outaddr + 0);
	LDA(addr1 + 1);
	SBC(addr2 + 1);
	STA(outaddr + 1);
}



}