
#include "process.h"

#include "method.h"
#include "context.h"

#define DEBUG_STEP(...) // do { printf(__VA_ARGS__); printf("\n");} while (0)

void Process::Start(const IMethod* method) {
    method->Begin(this);
}

void Process::PushFrame(const Method* method, int argCount) {
    stack.SetBase(0);
    locals.SetBase(0);

    Frame frame;
    memset(&frame, 0, sizeof(Frame));

    cur = frames.push(&frame);

    cur->method = method;
    cur->local = locals.GetTop();
    cur->pc = 0;
    cur->ret = 0;
    cur->stack = stack.GetTop() - argCount;

    stack.SetBase(cur->stack);
    locals.SetBase(cur->local);
}

void Process::Return() {
    int retCount = 0;
    Value retValue;
    if (cur != NULL) {
        retCount = cur->ret;
        if (retCount > 0) {
            retValue = *(stack.Pop());
        }
    }

    if (frames.size() == 0) {
        cur = NULL;
        return;
    }

    frames.pop();

    cur = frames.last();

    stack.Clear();
    locals.Clear();

    if (cur != NULL) {
        stack.SetBase(cur->stack);

        if (retCount > 0) {
            stack.Push(&retValue);
        }

        locals.SetBase(cur->local);
    }
}

void Process::StoreLocal(int pos, Value * obj) {
    if (locals.GetTop() <= pos) {
        locals.SetTop(pos + 1);
    }

    locals.Set(pos, obj);
}

bool Process::Step() {
    if (cur == NULL) {
        return false;
    }

    const Method* method = cur->method;
    IStack* stack = GetStack();

    int& pc = cur->pc;

    const Instruction * instruction = method->GetInstruction(pc);

    Code opcode = instruction->opcode;
    int64_t oprand = instruction->oprand;

    // method->Dump(this, pc);

    switch (instruction->opcode) {
    case Code::Ret:
        pc++; Return();
        break;
    case Code::Nop: 
        pc++;
        break;
    case Code::Break:
        pc = (int)oprand;
        break;
    case Code::Ldstr:
        stack->Push(context->GetString((int)oprand)); pc++;
        break;
    case Code::Call:
        pc++;
        CallMethod(oprand);
        break;
    case Code::Ldnull:
        stack->Push(Value::Nil); pc++;
        break;
    case Code::Ldc_I4_M1:
        stack->Push(Value((int)-1)); pc++;
        break;
    case Code::Ldc_I4_0:
        stack->Push(Value((int)0)); pc++;
        break;
    case Code::Ldc_I4_1:
        stack->Push(Value((int)1)); pc++;
        break;
    case Code::Ldc_I4_2:
        stack->Push(Value((int)2)); pc++;
        break;
    case Code::Ldc_I4_3:
        stack->Push(Value((int)3)); pc++;
        break;
    case Code::Ldc_I4_4:
        stack->Push(Value((int)4)); pc++;
        break;
    case Code::Ldc_I4_5:
        stack->Push(Value((int)5)); pc++;
        break;
    case Code::Ldc_I4_6:
        stack->Push(Value((int)6)); pc++;
        break;
    case Code::Ldc_I4_7:
        stack->Push(Value((int)7)); pc++;
        break;
    case Code::Ldc_I4_8:
        stack->Push(Value((int)8)); pc++;
        break;
    case Code::Ldc_I4_S:
        stack->Push((int)oprand); pc++;
        break;
    case Code::Ldc_I4:
        stack->Push((int)oprand); pc++;
        break;
    case Code::Ldc_I8:
        stack->Push((long)oprand); pc++;
        break;
    case Code::Ldc_R4:
        stack->Push((float)(*(double*)(&oprand))); pc++;
        break;
    case Code::Ldc_R8:
        stack->Push(*(double*)(&oprand)); pc++;
        break;
    case Code::Stloc_0:
        StoreLocal(0, stack->Pop()); pc++;
        break;
    case Code::Stloc_1:
        StoreLocal(1, stack->Pop()); pc++;
        break;
    case Code::Stloc_2:
        StoreLocal(2, stack->Pop()); pc++;
        break;
    case Code::Stloc_3:
        StoreLocal(3, stack->Pop()); pc++;
        break;
    case Code::Stloc_S:
        StoreLocal((int)oprand, stack->Pop()); pc++;
        break;
    case Code::Ldloc_0:
        stack->Push(*locals[0]); pc++;
        break;
    case Code::Ldloc_1:
        stack->Push(*locals[1]); pc++;
        break;
    case Code::Ldloc_2:
        stack->Push(*locals[2]); pc++;
        break;
    case Code::Ldloc_3:
        stack->Push(*locals[3]); pc++;
        break;
    case Code::Ldloc_S:
        stack->Push(locals[(int)oprand]); pc++;
        break;
    case Code::Br:
    case Code::Br_S:
        pc = (int)oprand;
        break;
    case Code::Pop:
        stack->Pop(); pc++;
        break;
    case Code::Ldarg_0:
        stack->Push(stack->Get(0)); pc++;
        break;
    case Code::Ldarg_1:
        stack->Push(stack->Get(1)); pc++;
        break;
    case Code::Ldarg_2:
        stack->Push(stack->Get(2)); pc++;
        break;
    case Code::Ldarg_3:
        stack->Push(stack->Get(3)); pc++;
        break;
    case Code::Ldarg_S:
        stack->Push(stack->Get((int)oprand)); pc++;
        break;
    case Code::Cgt: {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        stack->Push(v1->ToNumber() > v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Cgt_Un:
        assert(false);
        break;
    case Code::Ceq: {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        stack->Push(v1->ToNumber() == v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Clt:
    {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        stack->Push(v1->ToNumber() < v2->ToNumber() ? 1 : 0);
        pc++;
        break;
    }
    case Code::Clt_Un:
        assert(false);
        break;
    case Code::Brfalse:
    case Code::Brfalse_S:
        if (stack->Pop()->IsZero()) {
            pc = (int)oprand;
        }
        else {
            pc++;
        }
        break;
    case Code::Brtrue:
    case Code::Brtrue_S:
        if (!stack->Pop()->IsZero()) {
            pc = (int)oprand;
        }
        else {
            pc++;
        }
        break;
    case Code::Blt:
    case Code::Blt_S:
    {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        if (v1->ToNumber() < v2->ToNumber()) {
            pc = (int)oprand;
        }
        else {
            pc++;
        }
        break;
    }
        break;
    case Code::Add: {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        stack->Push(v1->Add(v2)); pc++;
    }
                  break;
    case Code::Sub: {
        Value* v2 = stack->Pop();
        Value* v1 = stack->Pop();
        stack->Push(v1->Sub(v2)); pc++;
    }
                  break;
    case Code::Mul: {
        Value* v2 = stack->Pop();
        Value* v1 = stack->Pop();
        stack->Push(v1->Mul(v2)); pc++;
    }
                  break;
    case Code::Div: {
        Value* v2 = stack->Pop();
        Value* v1 = stack->Pop();
        stack->Push(v1->Div(v2)); pc++;
        break;
    }
    case Code::Rem: {
        Value* v2 = stack->Pop();
        Value* v1 = stack->Pop();
        stack->Push(v1->Rem(v2)); pc++;
    }
    break;
    case Code::Neg:
    {
        stack->Push(stack->Pop()->Neg()); pc++;
        break;
    }
	case Code::Div_Un: {
        assert(false);
		Value* v2 = stack->Pop();
		Value* v1 = stack->Pop();
		uint32_t u1 = v1->ToInterger();
		uint32_t u2 = v2->ToInterger();
		stack->Push((int)(u1 / u2)); pc++;
	}
		break;
    case Code::Rem_Un:
        assert(false);
        break;
    case Code::And: {
        Value* v2 = stack->Pop();
        Value* v1 = stack->Pop();
        stack->Push(v1->Add(v2)); pc++;
        break;

    }
    case Code::Or: {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        stack->Push(v1->Or(v2)); pc++;
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
        stack->Push(locals[(int)oprand]); pc++;
        break;
    case Code::Ble_S: {
        Value * v2 = stack->Pop();
        Value * v1 = stack->Pop();
        if (v1->ToNumber() < v2->ToNumber()) {
            pc = (int)oprand;
        }
        else {
            pc++;
        }
        break;
    }
    case Code::Dup:
        stack->Push(stack->Pop()); pc++;
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

void Process::CallMethod(int64_t key)
{
    int ret = context->GetMethod(key)->Begin(this);

    //cur pointer changed

    cur->ret = ret;
}


#undef DEBUG_STEP