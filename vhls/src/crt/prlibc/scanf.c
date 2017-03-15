#define	BUF_NUMERIC	32 	/* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define	LONG		0x01	/* l: long or double */
#define	SHORT		0x04	/* h: short */
#define	SUPPRESS	0x08	/* *: suppress assignment */
#define	POINTER		0x10	/* p: void * (as hex) */
#define	NOSKIP		0x20	/* [ or c: do not skip blanks */
#define	LONGLONG	0x400	/* ll: long long (+ deprecated q: quad) */
#define	SHORTSHORT	0x4000	/* hh: char */
#define	UNSIGNED	0x8000	/* %[oupxX] conversions */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define	SIGNOK		0x40	/* +/- is (still) legal */
#define	NDIGITS		0x80	/* no digits detected */

#define	DPTOK		0x100	/* (float) decimal point is still legal */
#define	EXPOK		0x200	/* (float) exponent (e+3, etc) still legal */

#define	PFXOK		0x100	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x200	/* no zero digits detected */

/*
 * Conversion types.
 */
#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* %[dioupxX] conversion */

typedef long long quad_t;
typedef unsigned long long u_quad_t;

#define	UQUAD_MAX	((u_quad_t)0-1)	/* max value for a uquad_t */
					/* max value for a quad_t */
#define	QUAD_MAX	((quad_t)(UQUAD_MAX >> 1))
#define	QUAD_MIN	(-QUAD_MAX-1)	/* min value for a quad_t */

quad_t strtoq(const char *nptr, char **endptr, register int base) {
  register const char *s;
  register u_quad_t acc;
  register int c;
  register u_quad_t qbase, cutoff;
  register int neg, any, cutlim;
  
  /*
   * Skip white space and pick up leading +/- sign if any.
   * If base is 0, allow 0x for hex and 0 for octal, else
   * assume decimal; if base is already 16, allow 0x.
   */
  s = nptr;
  do {
    c = *s++;
  } while (isspace(c));
  if (c == '-') {
    neg = 1;
    c = *s++;
  } else {
    neg = 0;
    if (c == '+')
      c = *s++;
  }
  if ((base == 0 || base == 16) &&
      c == '0' && (*s == 'x' || *s == 'X')) {
    c = s[1];
    s += 2;
    base = 16;
  }
  if (base == 0)
    base = c == '0' ? 8 : 10;
  
  /*
   * Compute the cutoff value between legal numbers and illegal
   * numbers.  That is the largest legal value, divided by the
   * base.  An input number that is greater than this value, if
   * followed by a legal input character, is too big.  One that
   * is equal to this value may be valid or not; the limit
   * between valid and invalid numbers is then based on the last
   * digit.  For instance, if the range for quads is
   * [-9223372036854775808..9223372036854775807] and the input base
   * is 10, cutoff will be set to 922337203685477580 and cutlim to
   * either 7 (neg==0) or 8 (neg==1), meaning that if we have
   * accumulated a value > 922337203685477580, or equal but the
   * next digit is > 7 (or 8), the number is too big, and we will
   * return a range error.
   *
   * Set any if any `digits' consumed; make it negative to indicate
   * overflow.
   */
  qbase = (unsigned)base;
  cutoff = neg ? -(u_quad_t)QUAD_MIN : QUAD_MAX;
  cutlim = cutoff % qbase;
  cutoff /= qbase;
  for (acc = 0, any = 0;; c = *s++) {
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    else
      break;
    if (c >= base)
      break;
    if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
      any = -1;
    else {
      any = 1;
      acc *= qbase;
      acc += c;
    }
  }
  if (any < 0) {
    acc = neg ? QUAD_MIN : QUAD_MAX;
    errno = ERANGE;
  } else if (neg)
    acc = -acc;
  if (endptr != 0)
    *endptr = (char *)(any ? s - 1 : nptr);
  return (acc);
}

u_quad_t strtouq(const char *nptr, char **endptr, register int base) {
  register const char *s = nptr;
  register u_quad_t acc;
  register int c;
  register u_quad_t qbase, cutoff;
  register int neg, any, cutlim;
  
  /*
   * See strtoq for comments as to the logic used.
   */
  s = nptr;
  do {
    c = *s++;
  } while (isspace(c));
  if (c == '-') {
    neg = 1;
    c = *s++;
  } else { 
    neg = 0;
    if (c == '+')
      c = *s++;
  }
  if ((base == 0 || base == 16) &&
      c == '0' && (*s == 'x' || *s == 'X')) {
    c = s[1];
    s += 2;
    base = 16;
  }
  if (base == 0)
    base = c == '0' ? 8 : 10;
  qbase = (unsigned)base;
  cutoff = (u_quad_t)UQUAD_MAX / qbase;
  cutlim = (u_quad_t)UQUAD_MAX % qbase;
  for (acc = 0, any = 0;; c = *s++) {
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c -= isupper(c) ? 'A' - 10 : 'a' - 10;
    else
      break;
    if (c >= base)
      break;
    if (any < 0 || acc > cutoff || acc == cutoff && c > cutlim)
      any = -1;
    else {
      any = 1;
      acc *= qbase;
      acc += c;
    }
  }
  if (any < 0) {
    acc = UQUAD_MAX;
    errno = ERANGE;
  } else if (neg)
    acc = -acc;
  if (endptr != 0)
    *endptr = (char *)(any ? s - 1 : nptr);
  return (acc);
}

int vsscanf(const char *inp, char const *fmt0, VA_DECL_T ap) {
  int inr;
  const u_char *fmt = (const u_char *)fmt0;
  int c;			/* character from format, or conversion */
  size_t width;		/* field width, or 0 */
  char *p;		/* points into all kinds of strings */
  int n;			/* handy integer */
  int flags;		/* flags as defined above */
  char *p0;		/* saves original value of p when necessary */
  int nassigned;		/* number of fields assigned */
  int nconversions;	/* number of conversions */
  int nread;		/* number of characters consumed from fp */
  int base;		/* base argument to conversion function */
  char ccltab[256];	/* character class table for %[...] */
  char buf[BUF_NUMERIC];       /* buffer for numeric conversions */
  
  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static short basefix[17] =
    { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  
  inr = strlen(inp);
  
  nassigned = 0;
  nconversions = 0;
  nread = 0;
  base = 0;		/* XXX just to keep gcc happy */
  for (;;) {
    c = *fmt++;
    if (c == 0)
      return (nassigned);
    if (isspace(c)) {
      while (inr > 0 && isspace(*inp))
	nread++, inr--, inp++;
      continue;
    }
    if (c != '%')
      goto literal;
    width = 0;
    flags = 0;
    /*
     * switch on the format.  continue if done;
     * break once format type is derived.
     */
  again:		c = *fmt++;
    switch (c) {
    case '%':
    literal:
      if (inr <= 0)
	goto input_failure;
      if (*inp != c)
	goto match_failure;
      inr--, inp++;
      nread++;
      continue;
      
    case '*':
      flags |= SUPPRESS;
      goto again;
    case 'l':
      if (flags & LONG) {
	flags &= ~LONG;
	flags |= LONGLONG;
      } else
	flags |= LONG;
      goto again;
    case 'q':
      flags |= LONGLONG;	/* not quite */
      goto again;
    case 'h':
      if (flags & SHORT) {
	flags &= ~SHORT;
	flags |= SHORTSHORT;
      } else
	flags |= SHORT;
      goto again;
      
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      width = width * 10 + c - '0';
      goto again;
      
      /*
       * Conversions.
       */
    case 'd':
      c = CT_INT;
      base = 10;
      break;
      
    case 'i':
      c = CT_INT;
      base = 0;
      break;
      
    case 'o':
      c = CT_INT;
      flags |= UNSIGNED;
      base = 8;
      break;
      
    case 'u':
      c = CT_INT;
      flags |= UNSIGNED;
      base = 10;
      break;
      
    case 'X':
    case 'x':
      flags |= PFXOK;	/* enable 0x prefixing */
      c = CT_INT;
      flags |= UNSIGNED;
      base = 16;
      break;
      
    case 's':
      c = CT_STRING;
      break;
      
    case '[':
      fmt = __sccl(ccltab, fmt);
      flags |= NOSKIP;
      c = CT_CCL;
      break;
      
    case 'c':
      flags |= NOSKIP;
      c = CT_CHAR;
      break;
      
    case 'p':	/* pointer format is like hex */
      flags |= POINTER | PFXOK;
      c = CT_INT;
      flags |= UNSIGNED;
      base = 16;
      break;
      
    case 'n':
      nconversions++;
      if (flags & SUPPRESS)	/* ??? */
	continue;
      if (flags & SHORTSHORT)
	*PRLIBC_VA_ARG(ap, char *) = nread;
      else if (flags & SHORT)
	*PRLIBC_VA_ARG(ap, short *) = nread;
      else if (flags & LONG)
	*PRLIBC_VA_ARG(ap, long *) = nread;
      else if (flags & LONGLONG)
	*PRLIBC_VA_ARG(ap, long long *) = nread;
      else
	*PRLIBC_VA_ARG(ap, int *) = nread;
      continue;
    }
    
    /*
     * We have a conversion that requires input.
     */
    if (inr <= 0)
      goto input_failure;
    
    /*
     * Consume leading white space, except for formats
     * that suppress this.
     */
    if ((flags & NOSKIP) == 0) {
      while (isspace(*inp)) {
	nread++;
	if (--inr > 0)
	  inp++;
	else 
	  goto input_failure;
      }
      /*
       * Note that there is at least one character in
       * the buffer, so conversions that do not set NOSKIP
       * can no longer result in an input failure.
       */
    }
    
    /*
     * Do the conversion.
     */
    switch (c) {
      
    case CT_CHAR:
      /* scan arbitrary characters (sets NOSKIP) */
      if (width == 0)
	width = 1;
      if (flags & SUPPRESS) {
	size_t sum = 0;
	for (;;) {
	  if ((n = inr) < (int)width) {
	    sum += n;
	    width -= n;
	    inp += n;
	    if (sum == 0)
	      goto input_failure;
	    break;
	  } else {
	    sum += width;
	    inr -= width;
	    inp += width;
	    break;
	  }
	}
	nread += sum;
      } else {
	bcopy(inp, PRLIBC_VA_ARG(ap, char *), width);
	inr -= width;
	inp += width;
	nread += width;
	nassigned++;
      }
      nconversions++;
      break;
      
    case CT_CCL:
      /* scan a (nonempty) character class (sets NOSKIP) */
      if (width == 0)
	width = (size_t)~0;	/* `infinity' */
      /* take only those things in the class */
      if (flags & SUPPRESS) {
	n = 0;
	while (ccltab[(unsigned char)*inp]) {
	  n++, inr--, inp++;
	  if (--width == 0)
	    break;
	  if (inr <= 0) {
	    if (n == 0)
	      goto input_failure;
	    break;
	  }
	}
	if (n == 0)
	  goto match_failure;
      } else {
	p0 = p = PRLIBC_VA_ARG(ap, char *);
	while (ccltab[(unsigned char)*inp]) {
	  inr--;
	  *p++ = *inp++;
	  if (--width == 0)
	    break;
	  if (inr <= 0) {
	    if (p == p0)
	      goto input_failure;
	    break;
	  }
	}
	n = p - p0;
	if (n == 0)
	  goto match_failure;
	*p = 0;
	nassigned++;
      }
      nread += n;
      nconversions++;
      break;
      
    case CT_STRING:
      /* like CCL, but zero-length string OK, & no NOSKIP */
      if (width == 0)
	width = (size_t)~0;
      if (flags & SUPPRESS) {
	n = 0;
	while (!isspace(*inp)) {
	  n++, inr--, inp++;
	  if (--width == 0)
	    break;
	  if (inr <= 0)
	    break;
	}
	nread += n;
      } else {
	p0 = p = PRLIBC_VA_ARG(ap, char *);
	while (!isspace(*inp)) {
	  inr--;
	  *p++ = *inp++;
	  if (--width == 0)
	    break;
	  if (inr <= 0)
	    break;
	}
	*p = 0;
	nread += p - p0;
	nassigned++;
      }
      nconversions++;
      continue;
      
    case CT_INT:
      /* scan an integer as if by the conversion function */
#ifdef hardway
      if (width == 0 || width > sizeof(buf) - 1)
	width = sizeof(buf) - 1;
#else
      /* size_t is unsigned, hence this optimisation */
      if (--width > sizeof(buf) - 2)
	width = sizeof(buf) - 2;
      width++;
#endif
      flags |= SIGNOK | NDIGITS | NZDIGITS;
      for (p = buf; width; width--) {
	c = *inp;
	/*
	 * Switch on the character; `goto ok'
	 * if we accept it as a part of number.
	 */
	switch (c) {

	  /*
	   * The digit 0 is always legal, but is
	   * special.  For %i conversions, if no
	   * digits (zero or nonzero) have been
	   * scanned (only signs), we will have
	   * base==0.  In that case, we should set
	   * it to 8 and enable 0x prefixing.
	   * Also, if we have not scanned zero digits
	   * before this, do not turn off prefixing
	   * (someone else will turn it off if we
	   * have scanned any nonzero digits).
	   */
	case '0':
	  if (base == 0) {
	    base = 8;
	    flags |= PFXOK;
	  }
	  if (flags & NZDIGITS)
	    flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
	  else
	    flags &= ~(SIGNOK|PFXOK|NDIGITS);
	  goto ok;
	  
	  /* 1 through 7 always legal */
	case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	  base = basefix[base];
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* digits 8 and 9 ok iff decimal or hex */
	case '8': case '9':
	  base = basefix[base];
	  if (base <= 8)
	    break;	/* not legal here */
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* letters ok iff hex */
	case 'A': case 'B': case 'C':
	case 'D': case 'E': case 'F':
	case 'a': case 'b': case 'c':
	case 'd': case 'e': case 'f':
	  /* no need to fix base here */
	  if (base <= 10)
	    break;	/* not legal here */
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* sign ok only as first character */
	case '+': case '-':
	  if (flags & SIGNOK) {
	    flags &= ~SIGNOK;
	    goto ok;
	  }
	  break;
	  
	  /* x ok iff flag still set & 2nd char */
	case 'x': case 'X':
	  if (flags & PFXOK && p == buf + 1) {
	    base = 16;	/* if %i */
	    flags &= ~PFXOK;
	    goto ok;
	  }
	  break;
	}
	
	/*
	 * If we got here, c is not a legal character
	 * for a number.  Stop accumulating digits.
	 */
	break;
      ok:
	/*
	 * c is legal: store it and look at the next.
	 */
	*p++ = c;
	if (--inr > 0)
	  inp++;
	else 
	  break;		/* end of input */
      }
      /*
       * If we had only a sign, it is no good; push
       * back the sign.  If the number ends in `x',
       * it was [sign] '0' 'x', so push back the x
       * and treat it as [sign] '0'.
       */
      if (flags & NDIGITS) {
	if (p > buf) {
	  inp--;
	  inr++;
	}
	goto match_failure;
      }
      c = ((u_char *)p)[-1];
      if (c == 'x' || c == 'X') {
	--p;
	inp--;
	inr++;
      }
      if ((flags & SUPPRESS) == 0) {
	u_quad_t res;
	
	*p = 0;
	if ((flags & UNSIGNED) == 0)
	  res = strtoq(buf, (char **)NULL, base);
	else
	  res = strtouq(buf, (char **)NULL, base);
	if (flags & POINTER)
	  *PRLIBC_VA_ARG(ap, void **) =
	    (void *)(uintptr_t)res;
	else if (flags & SHORTSHORT)
	  *PRLIBC_VA_ARG(ap, char *) = res;
	else if (flags & SHORT)
	  *PRLIBC_VA_ARG(ap, short *) = res;
	else if (flags & LONG)
	  *PRLIBC_VA_ARG(ap, long *) = res;
	else if (flags & LONGLONG)
	  *PRLIBC_VA_ARG(ap, long long *) = res;
	else
	  *PRLIBC_VA_ARG(ap, int *) = res;
	nassigned++;
      }
      nread += p - buf;
      nconversions++;
      break;
      
    }
  }
 input_failure:
  return (nconversions != 0 ? nassigned : -1);
 match_failure:
  return (nassigned);
}
