#include "hires.h"

#include <debugf.h>
#include <hashembler.h>

#include <math.h>
#include "lib.h"
#include "memconfig.h"
#include "glm.h"

//#define BORDERTIME

using namespace hashembler;

segment_asm_c actualprg;

value_t irqfuk_begin = 0xF000;

void genis()
{
	CTX("irqf")

	actualprg.begin(irqfuk_begin);
	set_segment(&actualprg);

	LPC("begin");
	PRINTPOS("begin");

}


int main()
{
	srand(time(NULL));

	assemble(genis);

	list<segment_c *> segs;
	segs.push_back(&actualprg);
	make_prg("irqfukker.prg", irqfuk_begin, segs);
}