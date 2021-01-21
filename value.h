#ifndef _VM_VALUE_H_
#define _VM_VALUE_H_

#ifdef __cplusplus
extern "C" {
#endif

struct Field {
	int type;
	struct Value * value;
};

struct Value {
	int type;

	union {
		long Long;
		int Int;

		float Float;
		double Double;
	
		const char * str;

		struct Field * fields;
	} v;
};

#ifdef __cplusplus
}
#endif


#endif
