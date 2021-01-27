#include <string.h>

#include "process.h"

#include "method.h"
#include "context.h"




static void store_local(struct SimpleStack* stack, int index, Value* value) {
    int count = stack->head + stack->size - stack->base;
    if (count <= index) {
        grow_stack(stack, index - count + 1);
    }
    stack->base[index] = *value;
    if (stack->top < stack->base + index + 1) {
        stack->top = stack->base + index + 1;
    }
}

static void grow_ci(Process* p) {
    if (p->ci == p->base_ci + p->size_ci) {
        p->size_ci *= 2;
        
        size_t offset = p->ci - p->base_ci;

        p->base_ci = (CallInfo*)realloc(p->base_ci, sizeof(CallInfo) * p->size_ci);
        p->ci = p->base_ci + offset;
    }
}


void prepare_call(Process* p, const IMethod* method, int arg) {
    if (p->method != NULL) {
        grow_ci(p);

        p->ci->method = p->method;
        p->ci->local = p->local.base - p->local.head;
        p->ci->localCount = p->local.top - p->local.base;

        p->ci->stack = p->stack.base - p->stack.head;
        p->ci->stackCount = p->stack.top - p->stack.base - arg;
        assert(p->ci->stackCount >= 0);

        p->ci->pc = p->pc - p->method->GetInstruction(0);
        p->ci->ret = p->ret;

        p->ci++;
    }

    p->local.base = p->local.top;
    p->stack.base = p->stack.top - arg;
    p->method = method;
    p->pc = method->GetInstruction(0);
}

static void Return(Process* p) {
    if (p->ci == p->base_ci) {
        p->method = 0;
        return;
    }

    Value* ret = p->stack.top - 1;

    p->ci--;

    p->method = p->ci->method;
    p->stack.base = p->stack.head + p->ci->stack;
    p->stack.top = p->stack.base + p->ci->stackCount;

    p->local.base = p->local.head + p->ci->local;
    p->local.top = p->local.base + p->ci->localCount;
    p->pc = p->method->GetInstruction(p->ci->pc);

    if (p->ret > 0) {
        p->stack.push(ret);
    }

    p->ret = p->ci->ret;
}


static void dump_stack(SimpleStack* stack) {
    printf("---------\n");
    char c[256];
    for (Value* v = stack->top - 1; v >= stack->base; v--) {
        printf("  %s\n", v->ToString(c));
    }

    printf("---------\n");
}


#define stack_push(stack, t)  do { Value v(t); stack->push(&v); } while(0)

static bool step(Process* p) {
    if (p->method == 0) {
        return false;
    }

    const Context* context = p->context;
    const IMethod* method = p->method;

    Code opcode = p->pc->opcode;
    int64_t oprand = p->pc->oprand;

    SimpleStack* stack = &p->stack;
    SimpleStack* local = &p->local;

    // method->Dump(p, p->pc - method->GetInstruction(0));
    // dump_stack(stack);

    Instruction*& pc = p->pc;

    switch (opcode) {
    case Code::Ret:
        Return(p);
        break;
    case Code::Nop:
        pc++;
        break;
    case Code::Break:
        pc = method->GetInstruction((int)oprand);
        break;
    case Code::Ldstr:
        stack_push(stack, context->GetString((int)oprand)); pc++;
        break;
    case Code::Call: {
        pc++;
        int ret = context->GetMethodByIndex(oprand - 1)->Begin(p);
        p->ret = ret;
        break;
    }
    case Code::Ldnull:
        stack->push(&Value::Nil); pc++;
        break;
    case Code::Ldc_I4_M1:
        stack_push(stack, -1); pc++;
        break;
    case Code::Ldc_I4_0:
        stack_push(stack, 0); pc++;
        break;
    case Code::Ldc_I4_1:
        stack_push(stack, 1); pc++;
        break;
    case Code::Ldc_I4_2:
        stack_push(stack, 2); pc++;
        break;
    case Code::Ldc_I4_3:
        stack_push(stack, 3); pc++;
        break;
    case Code::Ldc_I4_4:
        stack_push(stack, 4); pc++;
        break;
    case Code::Ldc_I4_5:
        stack_push(stack, 5); pc++;
        break;
    case Code::Ldc_I4_6:
        stack_push(stack, 6); pc++;
        break;
    case Code::Ldc_I4_7:
        stack_push(stack, 7); pc++;
        break;
    case Code::Ldc_I4_8:
        stack_push(stack, 8); pc++;
        break;
    case Code::Ldc_I4_S:
        stack_push(stack,(int)oprand); pc++;
        break;
    case Code::Ldc_I4:
        stack_push(stack, (int)oprand); pc++;
        break;
    case Code::Ldc_I8:
        stack_push(stack, (long)oprand); pc++;
        break;
    case Code::Ldc_R4:
        stack_push(stack, (float)(*(double*)(&oprand))); pc++;
        break;
    case Code::Ldc_R8:
        stack_push(stack, *(double*)(&oprand)); pc++;
        break;
    case Code::Stloc_0:
        store_local(local, 0, stack->pop()); pc++;
        break;
    case Code::Stloc_1:
        store_local(local, 1, stack->pop()); pc++;
        break;
    case Code::Stloc_2:
        store_local(local, 2, stack->pop()); pc++;
        break;
    case Code::Stloc_3:
        store_local(local, 3, stack->pop()); pc++;
        break;
    case Code::Stloc_S:
        store_local(local, (int)oprand, stack->pop()); pc++;
        break;
    case Code::Ldloc_0:
        stack->push(local->get(0)); pc++;
        break;
    case Code::Ldloc_1:
        stack->push(local->get(1)); pc++;
        break;
    case Code::Ldloc_2:
        stack->push(local->get(2)); pc++;
        break;
    case Code::Ldloc_3:
        stack->push(local->get(3)); pc++;
        break;
    case Code::Ldloc_S:
        stack->push(local->get((int)oprand)); pc++;
        break;
    case Code::Br:
    case Code::Br_S:
        pc = method->GetInstruction(oprand);
        break;
    case Code::Pop:
        stack->pop(); pc++;
        break;
    case Code::Ldarg_0:
        stack->push(stack->get(0)); pc++;
        break;
    case Code::Ldarg_1:
        stack->push(stack->get(1)); pc++;
        break;
    case Code::Ldarg_2:
        stack->push(stack->get(2)); pc++;
        break;
    case Code::Ldarg_3:
        stack->push(stack->get(3)); pc++;
        break;
    case Code::Ldarg_S:
        stack->push(stack->get((int)oprand)); pc++;
        break;
    case Code::Cgt: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack_push(stack, v1->ToNumber() > v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Cgt_Un:
        assert(false);
        break;
    case Code::Ceq: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack_push(stack, v1->ToNumber() == v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Clt:
    {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack_push(stack, v1->ToNumber() < v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Clt_Un:
        assert(false);
        break;
    case Code::Brfalse:
    case Code::Brfalse_S:
        if (stack->pop()->IsZero()) {
            pc = method->GetInstruction(oprand);
        }
        else {
            pc++;
        }
        break;
    case Code::Brtrue:
    case Code::Brtrue_S:
        if (!stack->pop()->IsZero()) {
            pc = method->GetInstruction(oprand);
        }
        else {
            pc++;
        }
        break;
    case Code::Blt:
    case Code::Blt_S:
    {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        if (v1->ToNumber() < v2->ToNumber()) {
            pc = method->GetInstruction(oprand);
        }
        else {
            pc++;
        }
        break;
    }
    case Code::Add: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Add(v2)); pc++;
        break;
    }
    case Code::Sub: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Sub(v2)); pc++;
    }
                  break;
    case Code::Mul: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Mul(v2)); pc++;
        break;
    }
    case Code::Div: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Div(v2)); pc++;
        break;
    }
    case Code::Rem: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Rem(v2)); pc++;
        break;
    }
    case Code::Neg:
    {
        stack->push(stack->pop()->Neg()); pc++;
        break;
    }
    case Code::Div_Un: {
        assert(false);
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        uint32_t u1 = v1->ToInterger();
        uint32_t u2 = v2->ToInterger();
        stack_push(stack, (int)(u1 / u2)); pc++;
        break;
    }
    case Code::Rem_Un:
        assert(false);
        break;
    case Code::And: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Add(v2)); pc++;
        break;

    }
    case Code::Or: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        stack->push(v1->Or(v2)); pc++;
        break;
    }
    case Code::Xor:
    case Code::Shl:
    case Code::Shr:
    case Code::Not:
    case Code::Shr_Un:
        assert(false);
        break;
    case Code::Ldloca_S:
        stack->push(local->get(oprand)); pc++;
        break;
    case Code::Ble_S: {
        Value* v2 = stack->pop();
        Value* v1 = stack->pop();
        if (v1->ToNumber() < v2->ToNumber()) {
            pc = method->GetInstruction(oprand);
        }
        else {
            pc++;
        }
        break;
    }
    case Code::Dup:
        stack->push(stack->pop()); pc++;
        break;
    case Code::Jmp:
        assert(false);
        break;
    case Code::Ldarga_S:
    case Code::Calli:
    case Code::Starg_S:
    case Code::Beq_S:
    case Code::Bge_S:
    case Code::Bgt_S:
    case Code::Bne_Un_S:
    case Code::Bge_Un_S:
    case Code::Bgt_Un_S:
    case Code::Ble_Un_S:
    case Code::Blt_Un_S:
    case Code::Beq:
    case Code::Bge:
    case Code::Bgt:
    case Code::Ble:
    case Code::Bne_Un:
    case Code::Bge_Un:
    case Code::Bgt_Un:
    case Code::Ble_Un:
    case Code::Blt_Un:
    case Code::Switch:
    case Code::Ldind_I1:
    case Code::Ldind_U1:
    case Code::Ldind_I2:
    case Code::Ldind_U2:
    case Code::Ldind_I4:
    case Code::Ldind_U4:
    case Code::Ldind_I8:
    case Code::Ldind_I:
    case Code::Ldind_R4:
    case Code::Ldind_R8:
    case Code::Ldind_Ref:
    case Code::Stind_Ref:
    case Code::Stind_I1:
    case Code::Stind_I2:
    case Code::Stind_I4:
    case Code::Stind_I8:
    case Code::Stind_R4:
    case Code::Stind_R8:

    case Code::Conv_I1:
    case Code::Conv_I2:
    case Code::Conv_I4:
    case Code::Conv_I8:
    case Code::Conv_R4:
    case Code::Conv_R8:
    case Code::Conv_U4:
    case Code::Conv_U8:
    case Code::Callvirt:
    case Code::Cpobj:
    case Code::Ldobj:
    case Code::Newobj:
    case Code::Castclass:
    case Code::Isinst:
    case Code::Conv_R_Un:
    case Code::Unbox:
    case Code::Throw:
    case Code::Ldfld:
    case Code::Ldflda:
    case Code::Stfld:
    case Code::Ldsfld:
    case Code::Ldsflda:
    case Code::Stsfld:
    case Code::Stobj:
    case Code::Conv_Ovf_I1_Un:
    case Code::Conv_Ovf_I2_Un:
    case Code::Conv_Ovf_I4_Un:
    case Code::Conv_Ovf_I8_Un:
    case Code::Conv_Ovf_U1_Un:
    case Code::Conv_Ovf_U2_Un:
    case Code::Conv_Ovf_U4_Un:
    case Code::Conv_Ovf_U8_Un:
    case Code::Conv_Ovf_I_Un:
    case Code::Conv_Ovf_U_Un:
    case Code::Box:
    case Code::Newarr:
    case Code::Ldlen:
    case Code::Ldelema:
    case Code::Ldelem_I1:
    case Code::Ldelem_U1:
    case Code::Ldelem_I2:
    case Code::Ldelem_U2:
    case Code::Ldelem_I4:
    case Code::Ldelem_U4:
    case Code::Ldelem_I8:
    case Code::Ldelem_I:
    case Code::Ldelem_R4:
    case Code::Ldelem_R8:
    case Code::Ldelem_Ref:
    case Code::Stelem_I:
    case Code::Stelem_I1:
    case Code::Stelem_I2:
    case Code::Stelem_I4:
    case Code::Stelem_I8:
    case Code::Stelem_R4:
    case Code::Stelem_R8:
    case Code::Stelem_Ref:
    case Code::Ldelem_Any:
    case Code::Stelem_Any:
    case Code::Unbox_Any:
    case Code::Conv_Ovf_I1:
    case Code::Conv_Ovf_U1:
    case Code::Conv_Ovf_I2:
    case Code::Conv_Ovf_U2:
    case Code::Conv_Ovf_I4:
    case Code::Conv_Ovf_U4:
    case Code::Conv_Ovf_I8:
    case Code::Conv_Ovf_U8:
    case Code::Refanyval:
    case Code::Ckfinite:
    case Code::Mkrefany:
    case Code::Ldtoken:
    case Code::Conv_U2:
    case Code::Conv_U1:
    case Code::Conv_I:
    case Code::Conv_Ovf_I:
    case Code::Conv_Ovf_U:
    case Code::Add_Ovf:
    case Code::Add_Ovf_Un:
    case Code::Mul_Ovf:
    case Code::Mul_Ovf_Un:
    case Code::Sub_Ovf:
    case Code::Sub_Ovf_Un:
    case Code::Endfinally:
    case Code::Leave:
    case Code::Leave_S:
    case Code::Stind_I:
    case Code::Conv_U:
    case Code::Arglist:
    case Code::Ldftn:
    case Code::Ldvirtftn:
    case Code::Ldarg:
    case Code::Ldarga:
    case Code::Starg:
    case Code::Ldloc:
    case Code::Ldloca:
    case Code::Stloc:
    case Code::Localloc:
    case Code::Endfilter:
    case Code::Unaligned:
    case Code::Volatile:
    case Code::Tail:
    case Code::Initobj:
    case Code::Constrained:
    case Code::Cpblk:
    case Code::Initblk:
    case Code::No:
    case Code::Rethrow:
    case Code::Sizeof:
    case Code::Refanytype:
    case Code::Readonly:
    default:
        assert(false);
        pc++;
        break;
    }
    return true;
}

void run(const Context* context, int64_t key) {
    IMethod* m = context->GetMethod(key);
    assert(m);

    Process p;
    memset(&p, 0, sizeof(Process));

    p.context = context;
    p.base_ci = (CallInfo*)malloc(sizeof(CallInfo) * 4);
    p.size_ci = 4;
    p.ci = p.base_ci;

    init_stack(&p.local, 4);
    init_stack(&p.stack, 4);
    p.pc = m->GetInstruction(0);

    m->Begin(&p);

    while (step(&p)) {};
}
