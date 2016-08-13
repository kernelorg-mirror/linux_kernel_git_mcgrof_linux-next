#ifndef _ASM_X86_ASM_H

#ifdef __ASSEMBLY__
# define __ASM_FORM(x)	x
# define __ASM_FORM_RAW(x)     x
# define __ASM_FORM_COMMA(x) x,
#else
# define __ASM_FORM(x)	" " #x " "
# define __ASM_FORM_RAW(x)     #x
# define __ASM_FORM_COMMA(x) " " #x ","
#endif

# define __ASM_SEL(a,b) __ASM_FORM(b)
# define __ASM_SEL_RAW(a,b) __ASM_FORM_RAW(b)
#define _ASM_PTR	__ASM_SEL(.long, .quad)

#endif /* _ASM_X86_ASM_H */
