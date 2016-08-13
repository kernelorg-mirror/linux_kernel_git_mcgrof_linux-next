#include <asm/ranges.h>

	DEFINE_SECTION_RANGE(SECTION_TEXT, synth_init_or)

	push_section_rng_level(SECTION_TEXT, synth_init_or, "01", )
	/* err = 0; */
	pushq %rbp
	movq %rsp,%rbp
	pushq %rbx
	xorq %rbx,%rbx
	.popsection

	push_section_rng_level(SECTION_TEXT, synth_init_or, "99", )
	/* return err; */
	movq %rbx,%rax
	popq %rbx
	popq %rbp
	retq
	.popsection

	push_section_rng_level(SECTION_TEXT, synth_init_or, "50", )
	/* err |= demo_or_1(1); */
	movl $1,%edi
	call demo_or_1
	orq %rax,%rbx
	.popsection

	push_section_rng_level(SECTION_TEXT, synth_init_or, "50", )
	/* err |= demo_or_1(2); */
	movl $2,%edi
	call demo_or_1
	orq %rax,%rbx
	.popsection

	push_section_rng_level(SECTION_TEXT, synth_init_or, "50", )
	/* err |= demo_or_2(); */
	call demo_or_2
	orq %rax,%rbx
	.popsection
