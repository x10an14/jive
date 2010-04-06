#include <assert.h>

#include <jive/buffer.h>
#include <jive/internal/instruction_str.h>
#include <jive/i386/machine.h>
#include <jive/i386/abi.h>
#include <jive/passthrough.h>
#include <jive/regalloc.h>
#include <jive/graphdebug.h>
#include <jive/subroutine.h>

typedef long (*dotprod_function_t)(const int *, const int *);

dotprod_function_t
make_dotprod_function(size_t vector_size)
{
	jive_context * ctx = jive_context_create();
	jive_graph * graph = jive_graph_create(ctx);
	
	jive_subroutine * subroutine = jive_i386_subroutine_create(graph, 2, true);
	
	jive_value * p1 = jive_subroutine_parameter(subroutine, 0);
	jive_value * p2 = jive_subroutine_parameter(subroutine, 1);
	
	jive_value * operands[vector_size];
	size_t n;
	for(n=0; n<vector_size; n++) {
		long displacement = n * 4;
		jive_node * a1 = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p1, &displacement);
		jive_value * v1 = jive_instruction_output(a1, 0);
		jive_node * a2 = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_load32_disp],
			&p2, &displacement);
		jive_value * v2 = jive_instruction_output(a2, 0);
		
		jive_node * m = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_mul],
			(jive_value *[]){v1, v2}, 0);
		operands[n] = jive_instruction_output(m, 0);
	}
	
	jive_value * value = operands[0];
	for(n=1; n<vector_size; n++) {
		jive_node * s = jive_instruction_create(graph,
			&jive_i386_instructions[jive_i386_int_add],
			(jive_value *[]){value, operands[n]}, 0);
		value = jive_instruction_output(s, 0);
	}
	
	jive_subroutine_return_value(subroutine, value);
	
	jive_regalloc(graph, &jive_i386_machine, 0);
	
	jive_instruction_sequence seq;
	jive_graph_sequentialize(graph, &seq);
	
	jive_buffer buffer;
	jive_buffer_init(&buffer);
	jive_instruction_sequence_encode(&seq, &buffer, &jive_i386_machine);
	
	dotprod_function_t function = (dotprod_function_t) jive_buffer_executable(&buffer);
	
	jive_buffer_destroy(&buffer);
	
	jive_context_destroy(ctx);
	
	return function;
}


int main()
{
	dotprod_function_t dotprod2 = make_dotprod_function(2);
	
	int a[] = {1, 4};
	int b[] = {3, 7};
	int result = dotprod2(a, b);
	assert(result = 1*3 + 4*7);
	
	return 0;
}

