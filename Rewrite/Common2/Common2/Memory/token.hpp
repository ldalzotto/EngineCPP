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

template<class ElementType>
struct Token
{
	token_t tok;
};

template<class ElementType>
inline Token<ElementType> token_build_default()
{
	return Token<ElementType>{cast(token_t, -1)};
};

#define token_t_v(SouceToken) SouceToken.tok
#define token_t_p(SouceToken) &(SouceToken)->tok

#define token_cast_v(TargetType, SourceToken) Token<##TargetType##>{(SourceToken).tok} 
#define token_cast_p(TargetType, SourceToken) (Token<##TargetType##>*)(SourceToken)

#define Token(ElementType) Token<##ElementType##>