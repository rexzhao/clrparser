
#include "process.h"

#include "method.h"
#include "context.h"

#define DEBUG_STEP(...) // do { printf(__VA_ARGS__); printf("\n");} while (0)

void Process::Start(const IMethod* method) {
    method->Begin(this);
}

void Process::Return() {
    int retCount = 0;
    Value retValue;
    if (cur != NULL) {
        retCount = cur->ret;
        if (retCount > 0) {
            retValue = cur->stack->Pop();
        }
        delete cur;
    }

    if (frames.empty()) {
        cur = NULL;
    }
    else {
        cur = frames.back();
        frames.pop_back();

        if (retCount > 0) {
            cur->stack->Push(retValue);
        }
    }
}

void Process::StoreLocal(int pos, const Value& obj) {
    int n = cur->locals.size();
    if (n <= pos) {
        if (n <= 0) n = 4;
        while (n <= pos) { n *= 2; }
        cur->locals.resize(n);
    }

    cur->locals[pos] = obj;
}

bool Process::Step() {
    if (cur == NULL) {
        return false;
    }

    const Method* method = cur->method;
    IStack* stack = cur->stack;

    int& pc = cur->pc;

    Instruction instruction = method->GetInstruction(pc);
    switch (instruction.opcode) {
    case Code::Ret:
        DEBUG_STEP("ret %d", cur->ret);
        pc++; Return();
        break;
    case Code::Nop: 
        DEBUG_STEP("nop");
        pc++;
        break;
    case Code::Break:
        pc = (int)instruction.oprand;
        DEBUG_STEP("break %d", pc);
        break;
    case Code::Ldstr:
        DEBUG_STEP("ldstr %s", context->GetString((int)instruction.oprand));
        stack->Push(context->GetString((int)instruction.oprand)); pc++;
        break;
    case Code::Call:
        pc++;
        CallMethod(instruction.oprand);
        break;
    case Code::Ldnull:
        DEBUG_STEP("ldnull");
        stack->Push(Value::Nil); pc++;
        break;
    case Code::Ldc_I4_M1:
        DEBUG_STEP("ldc.i4.m1");
        stack->Push(Value((int)-1)); pc++;
        break;
    case Code::Ldc_I4_0:
        DEBUG_STEP("ldc.i4.0");
        stack->Push(Value((int)0)); pc++;
        break;
    case Code::Ldc_I4_1:
        DEBUG_STEP("ldc.i4.1");
        stack->Push(Value((int)1)); pc++;
        break;
    case Code::Ldc_I4_2:
        DEBUG_STEP("ldc.i4.2");
        stack->Push(Value((int)2)); pc++;
        break;
    case Code::Ldc_I4_3:
        DEBUG_STEP("ldc.i4.3");
        stack->Push(Value((int)3)); pc++;
        break;
    case Code::Ldc_I4_4:
        DEBUG_STEP("ldc.i4.4");
        stack->Push(Value((int)4)); pc++;
        break;
    case Code::Ldc_I4_5:
        DEBUG_STEP("ldc.i4.5");
        stack->Push(Value((int)5)); pc++;
        break;
    case Code::Ldc_I4_6:
        DEBUG_STEP("ldc.i4.6");
        stack->Push(Value((int)6)); pc++;
        break;
    case Code::Ldc_I4_7:
        DEBUG_STEP("ldc.i4.7");
        stack->Push(Value((int)7)); pc++;
        break;
    case Code::Ldc_I4_8:
        DEBUG_STEP("ldc.i4.8");
        stack->Push(Value((int)8)); pc++;
        break;
    case Code::Ldc_I4_S:
        DEBUG_STEP("ldc.i4.s %d", (int)instruction.oprand);
        stack->Push((int)instruction.oprand); pc++;
        break;
    case Code::Ldc_I4:
        DEBUG_STEP("ldc.i4 %d", (int)instruction.oprand);
        stack->Push((int)instruction.oprand); pc++;
        break;
    case Code::Ldc_I8:
        DEBUG_STEP("ldc.i4 %ld", (long)instruction.oprand);
        stack->Push((long)instruction.oprand); pc++;
        break;
    case Code::Ldc_R4:
        DEBUG_STEP("ldc.r4 %f", (float)(*(double*)(&instruction.oprand)));
        stack->Push((float)(*(double*)(&instruction.oprand))); pc++;
        break;
    case Code::Ldc_R8:
        DEBUG_STEP("ldc.r4 %lf", (*(double*)(&instruction.oprand)));
        stack->Push(*(double*)(&instruction.oprand)); pc++;
        break;
    case Code::Stloc_0:
        DEBUG_STEP("stloc.0");
        StoreLocal(0, stack->Pop()); pc++;
        break;
    case Code::Stloc_1:
        DEBUG_STEP("stloc.1");
        StoreLocal(1, stack->Pop()); pc++;
        break;
    case Code::Stloc_2:
        DEBUG_STEP("stloc.2");
        StoreLocal(2, stack->Pop()); pc++;
        break;
    case Code::Stloc_3:
        DEBUG_STEP("stloc.3");
        StoreLocal(3, stack->Pop()); pc++;
        break;
    case Code::Stloc_S:
        DEBUG_STEP("stloc.s %d", (int)instruction.oprand);
        StoreLocal((int)instruction.oprand, stack->Pop()); pc++;
        break;
    case Code::Ldloc_0:
        DEBUG_STEP("ldloc.0");
        stack->Push(cur->locals[0]); pc++;
        break;
    case Code::Ldloc_1:
        DEBUG_STEP("ldloc.1");
        stack->Push(cur->locals[1]); pc++;
        break;
    case Code::Ldloc_2:
        DEBUG_STEP("ldloc.2");
        stack->Push(cur->locals[2]); pc++;
        break;
    case Code::Ldloc_3:
        DEBUG_STEP("ldloc.3");
        stack->Push(cur->locals[3]); pc++;
        break;
    case Code::Ldloc_S:
        DEBUG_STEP("ldloc.s %d", (int)instruction.oprand);
        stack->Push(cur->locals[(int)instruction.oprand]); pc++;
        break;
    case Code::Br:
    case Code::Br_S:
        DEBUG_STEP("br %d", (int)instruction.oprand);
        pc = (int)instruction.oprand;
        break;
    case Code::Pop:
        DEBUG_STEP("pop");
        stack->Pop(); pc++;
        break;
    case Code::Ldarg_0:
        DEBUG_STEP("ldarg.0");
        stack->Push(stack->Get(0)); pc++;
        break;
    case Code::Ldarg_1:
        DEBUG_STEP("ldarg.1");
        stack->Push(stack->Get(1)); pc++;
        break;
    case Code::Ldarg_2:
        DEBUG_STEP("ldarg.2");
        stack->Push(stack->Get(2)); pc++;
        break;
    case Code::Ldarg_3:
        DEBUG_STEP("ldarg.3");
        stack->Push(stack->Get(3)); pc++;
        break;
    case Code::Ldarg_S:
        DEBUG_STEP("ldarg.s %d", (int)instruction.oprand);
        stack->Push(stack->Get((int)instruction.oprand)); pc++;
        break;
    case Code::Cgt: {
        DEBUG_STEP("cgt");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1.ToNumber() > v2.ToNumber() ? 1 : 0);
        pc++;
    }
                  break;
    case Code::Brfalse:
    case Code::Brfalse_S:
        DEBUG_STEP("brfalse");
        if (stack->Pop().IsZero()) {
            pc = (int)instruction.oprand;
        }
        else {
            pc++;
        }
        break;
    case Code::Brtrue:
    case Code::Brtrue_S:
        DEBUG_STEP("brtrue");
        if (!stack->Pop().IsZero()) {
            pc = (int)instruction.oprand;
        }
        else {
            pc++;
        }
        break;
        // TODO:
    case Code::Add: {
        DEBUG_STEP("add");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 + v2); pc++;
    }
                  break;
    case Code::Sub: {
        DEBUG_STEP("sub");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 - v2); pc++;
    }
                  break;
    case Code::Mul: {
        DEBUG_STEP("mul");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 * v2); pc++;
    }
                  break;
    case Code::Div: {
        DEBUG_STEP("div");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 / v2); pc++;
    }
                  break;
    case Code::Rem: {
        DEBUG_STEP("rem");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 % v2); pc++;
    }
    break;
    case Code::Neg:
    {
        DEBUG_STEP("neg");
        stack->Push(-stack->Pop()); pc++;
    }
    break;
	case Code::Div_Un: {
        assert(false);
		DEBUG_STEP("div.un");
		Value v2 = stack->Pop();
		Value v1 = stack->Pop();
		uint32_t u1 = v1.ToInterger();
		uint32_t u2 = v2.ToInterger();
		stack->Push((int)(u1 / u2)); pc++;
	}
		break;
    case Code::Rem_Un:
        assert(false);
        break;
    case Code::And: {
        DEBUG_STEP("and");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 & v2); pc++;
    }
                  break;
    case Code::Or: {
        DEBUG_STEP("or");
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        stack->Push(v1 | v2); pc++;
    }
                 break;
    case Code::Xor:
    case Code::Shl:
    case Code::Shr:
    case Code::Not:
    case Code::Shr_Un:
        assert(false);
        break;
    case Code::Ldloca_S:
        DEBUG_STEP("ldloca.s %d", (int)instruction.oprand);
        stack->Push(cur->locals[(int)instruction.oprand]); pc++;
        break;
    case Code::Ble_S: {
        DEBUG_STEP("ble.s %d", (int)instruction.oprand);
        Value v2 = stack->Pop();
        Value v1 = stack->Pop();
        if (v1.ToNumber() < v2.ToNumber()) {
            pc = (int)instruction.oprand;
        }
        else {
            pc++;
        }
    }
                    break;
    case Code::Dup:
        DEBUG_STEP("dup");
        stack->Push(stack->Pop()); pc++;
        break;
    case Code::Jmp:
        assert(false);
        DEBUG_STEP("jmp");
        break;
    case Code::Ldarga_S:
    case Code::Calli:
    case Code::Starg_S:
    case Code::Beq_S:
    case Code::Bge_S:
    case Code::Bgt_S:
    case Code::Blt_S:
    case Code::Bne_Un_S:
    case Code::Bge_Un_S:
    case Code::Bgt_Un_S:
    case Code::Ble_Un_S:
    case Code::Blt_Un_S:
    case Code::Beq:
    case Code::Bge:
    case Code::Bgt:
    case Code::Ble:
    case Code::Blt:
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
    case Code::Ceq:
    case Code::Cgt_Un:
    case Code::Clt:
    case Code::Clt_Un:
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
    DEBUG_STEP("call %s", context->GetMemberName(key).c_str());
    int ret = context->GetMethod(key)->Begin(this);

    //cur pointer changed

    cur->ret = ret;
}


#undef DEBUG_STEP