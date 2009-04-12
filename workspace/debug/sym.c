#include <cda/c_string.h>
#include <cda/c_stack.h>

typedef enum symbol_type
{
	NUM=0,
	LK=1,
	ADD=2,
	SUB=3,
	MUL=4,
	DIV=5
}SYMBOL_TYPE;

typedef struct exp_symbol
{
	SYMBOL_TYPE type;
	UINT_ num;
}EXP_SYMBOL;

void c_Calculation(CHAR_ *exp_string);

void c_Calculation(CHAR_ *exp_string)
{
	C_STRING str_cache;
	CHAR_ *op;
	EXP_SYMBOL *in_stack;
	EXP_SYMBOL *out_stack;
	EXP_SYMBOL *num_stack;
	EXP_SYMBOL tmp;
	UINT_ a;
	UINT_ b;

	str_cache=string_Create();
	in_stack=stack_Create(sizeof(EXP_SYMBOL));
	out_stack=stack_Create(sizeof(EXP_SYMBOL));

	for(op=exp_string;*op!=ENCODE_('\0');++op)
	{
		if(!ISDIGIT_(*op))
		{
			switch(*op)
			{
				case ENCODE_('+'):tmp.type=ADD;break;
				case ENCODE_('-'):tmp.type=SUB;break;
				case ENCODE_('*'):tmp.type=MUL;break;
				case ENCODE_('/'):tmp.type=DIV;break;
				case ENCODE_('('):tmp.type=LK;break;
				case ENCODE_(')'):
				{
					while(stack_Get(in_stack)->type!=LK)
					{
						out_stack=stack_Append(out_stack,stack_Get(in_stack));
						in_stack=stack_Remove(in_stack);
					};
					in_stack=stack_Remove(in_stack);
					continue;
					break;
				}
				default:
				{
					goto fail_return;
					break;
				};
			};
			if(array_Length(in_stack)>0&&tmp.type<stack_Get(in_stack)->type&&tmp.type!=LK)
			{
				while(stack_Get(in_stack)->type>tmp.type)
				{
					out_stack=stack_Append(out_stack,stack_Get(in_stack));
					in_stack=stack_Remove(in_stack);
				};
			};
			in_stack=stack_Append(in_stack,&tmp);
		}
		else
		{
			str_cache=array_Resize(str_cache,0);
			for(;ISDIGIT_(*op);++op)str_cache=array_Append(str_cache,op);
			tmp.type=NUM;
			tmp.num=STRTOL_(str_cache,NULL,10);
			out_stack=stack_Append(out_stack,&tmp);
			--op;
		};
	};
	while(array_Length(in_stack)>0)
	{
		out_stack=stack_Append(out_stack,stack_Get(in_stack));
		in_stack=stack_Remove(in_stack);
	};

	num_stack=stack_Create(sizeof(EXP_SYMBOL));
	while(array_Length(out_stack)>0)
	{
		switch(queue_Get(out_stack)->type)
		{
			case NUM:
			{
				num_stack=stack_Append(num_stack,queue_Get(out_stack));
				break;
			}
			case ADD:
			{
				b=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				a=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				tmp.type=NUM;
				tmp.num=a+b;
				num_stack=stack_Append(num_stack,&tmp);
				break;
			}
			case SUB:
			{
				b=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				a=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				tmp.type=NUM;
				tmp.num=a-b;
				num_stack=stack_Append(num_stack,&tmp);
				break;
			}
			case MUL:
			{
				b=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				a=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				tmp.type=NUM;
				tmp.num=a*b;
				num_stack=stack_Append(num_stack,&tmp);
				break;
			}
			case DIV:
			{
				b=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				a=stack_Get(num_stack)->num;
				num_stack=stack_Remove(num_stack);
				tmp.type=NUM;
				tmp.num=a/b;
				num_stack=stack_Append(num_stack,&tmp);
				break;
			}
			default:goto fail_return;break;
		};
		out_stack=queue_Remove(out_stack);
	};
	PRINTF_(ENCODE_("result is:%lu\n"),stack_Get(num_stack)->num);
	string_Drop(str_cache);
	stack_Drop(in_stack);
	stack_Drop(out_stack);

	return;

	fail_return:
	PRINTF_(ENCODE_("FAILED\n"));
	string_Drop(str_cache);
	stack_Drop(in_stack);
	stack_Drop(out_stack);

};

int main()
{
	char exp_ansi[2048];
	CHAR_ *exp;

	scanf("%s",exp_ansi);
	exp=string_Create();
	string_Ansitowide(&exp,exp_ansi);
	
	c_Calculation(exp);
	return 0;
};
