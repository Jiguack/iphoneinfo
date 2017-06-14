#ifdef MATHFUNCSDLL_EXPORTS
#define MATHFUNCSDLL_API __declspec(dllexport)
#else
#define MATHFUNCSDLL_API __declspec(dllimport)
#endif


	MATHFUNCSDLL_API int icount(void);
	MATHFUNCSDLL_API int ilist(void);
	MATHFUNCSDLL_API void ibattery(void);
	MATHFUNCSDLL_API void ibattery_id(const char* id);
	MATHFUNCSDLL_API void iinfo(void);
	MATHFUNCSDLL_API void iinfo_id(const char* id);