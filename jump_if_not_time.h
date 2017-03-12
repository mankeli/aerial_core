void jump_if_not_time(value_t newtime, value_t pos)
{
	LDAz(framecounter+0)
	CMPi(LB(newtime))
	BNE(pos)
	LDAz(framecounter+1)
	CMPi(HB(newtime))
	BNE(pos)
/*
	LDA framecounter+0
	SEC
	SBC #<newtime
	LDA framecounter+1
	SBC #>newtime
	BCS pos
*/
}