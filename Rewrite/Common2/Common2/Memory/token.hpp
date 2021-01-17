#pragma once

typedef size_t token_t;

inline token_t tokent_build_default()
{
	return -1;
};

inline void tokent_reset(token_t* p_token)
{
	*p_token = tokent_build_default();
};

inline char tokent_is_valid(const token_t* p_token)
{
	return *p_token != -1;
};

#if TOKEN_TYPE_SAFETY

template<class ElementType>
struct Token
{
	token_t tok;
};

#define tk_b(ElementType, TokenT) Token<ElementType> { (token_t)TokenT }

#define tk_v(TokenVariable) ((TokenVariable).tok)

#define Token(ElementType) Token<ElementType>

#else

#define tk_b(ElementType, TokenT) (token_t)TokenT

#define tk_v(TokenVariable) (TokenVariable)

#define Token(ElementType) size_t

#endif

#define tk_bd(ElementType) tk_b(ElementType, -1)
#define tk_bf(ElementType, SourceToken) tk_b(ElementType, tk_v(SourceToken))
#define tk_eq(Left, Right) (tk_v(Left) == tk_v(Right))
#define tk_neq(Left, Right) (tk_v(Left) != tk_v(Right))