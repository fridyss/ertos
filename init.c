#include "init.h"

static int init_start(void)
{
	return 0;
}
INIT_EXPORT(init_start, "0");

static int init_borad_start()
{
	return 0;
}
INIT_EXPORT(init_borad_start, "0.end");

static int init_borad_end(void)
{
	return 0;
}
INIT_EXPORT(init_borad_end, "1.end");


static int init_end(void)
{
	return 0;
}
INIT_EXPORT(init_end, "6.end");

extern init_fun_t _symbol_init_init_borad_start;
extern init_fun_t _symbol_init_init_borad_end;
extern init_fun_t _symbol_init_init_end;

/*开发板相关初始化*/
void borad_init(void)
{
	const init_fun_t *fn_ptr;
	for( fn_ptr =  &_symbol_init_init_borad_start; 		\
				fn_ptr < &_symbol_init_init_borad_end;		\
				fn_ptr++ )
	{
		(*fn_ptr)();
	}
}

/*应用相关初始化*/
void app_init(void)
{

	const init_fun_t *fn_ptr;
	for( fn_ptr = &_symbol_init_init_borad_end;		\
				fn_ptr = &_symbol_init_init_end;		\
				fn_ptr++ )
	{
		(*fn_ptr)();
	}
}


