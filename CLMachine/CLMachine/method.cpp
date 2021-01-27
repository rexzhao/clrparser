#include "method.h"

#include "context.h"
#include "stack.h"
#include "process.h"

#include <iostream>

int Method::Begin(Process* p)  const
{
    p->PushFrame(this, argCount >> 1);
    return argCount & 1;
}


void Method::SetInstruction(Instruction * instructions, int count) {
    this->instructions = instructions;
    this->instructinsCount = count;
}

Instruction* Method::GetInstruction(int i) const
{
    if (i < instructinsCount) {
        return instructions + i;
    }
    return NULL;
}



void Method::DumpInstruction(Process* process, const Instruction& instruction) const {
    const Context* context = process->GetContext();
    switch (instruction.opcode) {
    case Code::Ret:        printf("ret");                                                               break;
    case Code::Nop:        printf("nop");                                                               break;
    case Code::Break:      printf("break %d", (int)instruction.oprand);                             break;
    case Code::Ldstr:      printf("ldstr %s", context->GetString((int)instruction.oprand));         break;
    case Code::Call:       printf("call %s", context->GetMemberName(instruction.oprand).c_str());  break;
    case Code::Ldnull:     printf("ldnull");                                                            break;
    case Code::Ldc_I4_M1:  printf("ldc.i4.m1");                                                         break;
    case Code::Ldc_I4_0:   printf("ldc.i4.0");                                                          break;
    case Code::Ldc_I4_1:   printf("ldc.i4.1");                                                          break;
    case Code::Ldc_I4_2:   printf("ldc.i4.2");                                                          break;
    case Code::Ldc_I4_3:   printf("ldc.i4.3");                                                          break;
    case Code::Ldc_I4_4:   printf("ldc.i4.4");                                                          break;
    case Code::Ldc_I4_5:   printf("ldc.i4.5");                                                          break;
    case Code::Ldc_I4_6:   printf("ldc.i4.6");                                                          break;
    case Code::Ldc_I4_7:   printf("ldc.i4.7");                                                          break;
    case Code::Ldc_I4_8:   printf("ldc.i4.8");                                                          break;
    case Code::Ldc_I4_S:   printf("ldc.i4.s %d",  (int)instruction.oprand);                            break;
    case Code::Ldc_I4:     printf("ldc.i4 %d",  (int)instruction.oprand);                            break;
    case Code::Ldc_I8:     printf("ldc.i8 %ld", (long)instruction.oprand);                           break;
    case Code::Ldc_R4:     printf("ldc.r4 %f",  (float)(*(double*)(&instruction.oprand)));           break;
    case Code::Ldc_R8:     printf("ldc.r8 %lf", (*(double*)(&instruction.oprand)));                  break;
    case Code::Stloc_0:    printf("stloc.0"); break;
    case Code::Stloc_1:    printf("stloc.1"); break;
    case Code::Stloc_2:    printf("stloc.2"); break;
    case Code::Stloc_3:    printf("stloc.3"); break;
    case Code::Stloc_S:    printf("stloc.s   %d", (int)instruction.oprand); break;
    case Code::Ldloc_0:    printf("ldloc.0"); break;
    case Code::Ldloc_1:    printf("ldloc.1"); break;
    case Code::Ldloc_2:    printf("ldloc.2"); break;
    case Code::Ldloc_3:    printf("ldloc.3"); break;
    case Code::Ldloc_S:    printf("ldloc.s   %d", (int)instruction.oprand); break;
    case Code::Br:         printf("br %d", (int)instruction.oprand); break;
    case Code::Br_S:       printf("br.s %d", (int)instruction.oprand); break;
    case Code::Pop:        printf("pop"); break;
    case Code::Ldarg_0:    printf("ldarg.0"); break;
    case Code::Ldarg_1:    printf("ldarg.1"); break;
    case Code::Ldarg_2:    printf("ldarg.2"); break;
    case Code::Ldarg_3:    printf("ldarg.3"); break;
    case Code::Ldarg_S:    printf("ldarg.s %d", (int)instruction.oprand); break;
    case Code::Cgt:        printf("cgt"); break;
    case Code::Brfalse:    printf("brfalse %d", (int)instruction.oprand); break;
    case Code::Brfalse_S:  printf("brfalse.s %d", (int)instruction.oprand); break;
    case Code::Brtrue:     printf("brtrue %d", (int)instruction.oprand); break;
    case Code::Brtrue_S:   printf("brtrue.s %d", (int)instruction.oprand);  break;
    case Code::Add:        printf("add"); break;
    case Code::Sub:        printf("sub");        break;
    case Code::Mul:        printf("mul");        break;
    case Code::Div:        printf("div");        break;
    case Code::Rem:        printf("rem");        break;
    case Code::Neg:        printf("neg");        break;
    case Code::Div_Un:     printf("div.un");     break;
    case Code::Rem_Un:
        assert(false);
        break;
    case Code::And:        printf("and");        break;
    case Code::Or:         printf("or");         break;
    case Code::Xor:
    case Code::Shl:
    case Code::Shr:
    case Code::Not:
    case Code::Shr_Un:
        assert(false);
        break;
    case Code::Ldloca_S:   printf("ldloca.s %d", (int)instruction.oprand);        break;
    case Code::Ble_S:      printf("ble.s %d", (int)instruction.oprand);        break;
    case Code::Dup:        printf("dup");                                          break;
    case Code::Jmp:
        assert(false);
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
        break;
    }
}

void Method::Dump(Process * process, int pc) const {
    std::cout << "=== " << process->GetContext()->GetMemberName(this->key) << " ===" << std::endl;
    for (int i = 0; i < instructinsCount; i++) {
        printf("%s %4d ", (i == pc) ? "-> " : "   ", i);
        DumpInstruction(process, instructions[i]);
        printf("\n");
    }
}


