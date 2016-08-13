int demo_or_1(int arg)
{
	switch (arg) {
	case 1:
		return 0xDEA00000;
	case 2:
		return 0X000D0000;
	default:
		return arg * 2;
	}
}

int demo_or_2(void)
{
	return 0x0000BEEF;
}
