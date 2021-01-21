using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

using Mono.Cecil;
using Mono.Cecil.Cil;

namespace CLRExport
{
    class Reference<T> : IEnumerable<T>
    {
        List<T> list = new List<T>();

        public int Get(T v, bool insert = false)
        {
            var index = list.FindIndex((o) => o.Equals(v));

            if (index < 0)
            {
                if (!insert)
                {
                    return 0;
                }

                list.Add(v);
                return list.Count;
            }

            return index + 1;
        }

        public int Add(T v)
        {
            return Get(v, true);
        }

        public IEnumerator<T> GetEnumerator()
        {
            return list.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return list.GetEnumerator();
        }

        public int Count
        {
            get { return list.Count; }
        }

        public T this[int index]
        {
            get
            {
                return list[index];
            }
        }

        public T[] ToArray()
        {
            return list.ToArray();
        }
    }

    enum TableType
    {
        METHOD,
        STRING,
        BLOB,
    }


    enum OperandType
    {
        NONE,
        VALUE,
        ARRAY,
    }

    class Context
    {
        public Reference<string> strings = new Reference<string>();
        public List<byte[]> blobs = new List<byte[]>();
        public System.IO.BinaryWriter writer;
    }

    public class Util
    {
        public static void Export(string Namespace)
        {
            Context context = new Context();
            context.writer = new System.IO.BinaryWriter(System.IO.File.Open("out.txt", System.IO.FileMode.Create));

            context.writer.Write((short)1);

            var assembly = Mono.Cecil.AssemblyDefinition.ReadAssembly(System.Reflection.Assembly.GetExecutingAssembly().Location);

            foreach (var m in assembly.Modules)
            {
                foreach (var t in m.Types)
                {
                    if (t.Namespace == Namespace)
                    {
                        Collect(context, t);
                    }
                }
            }

            context.writer.Write((byte)TableType.STRING);
            context.writer.Write(System.BitConverter.GetBytes(context.strings.Count));
            for (int i = 0; i < context.strings.Count; i++)
            {
                var s = context.strings[i];
                var bs = System.Text.Encoding.UTF8.GetBytes(s);
                context.writer.Write(System.BitConverter.GetBytes(bs.Length));
                context.writer.Write(bs);
            }

            context.writer.Write((byte)TableType.BLOB);
            context.writer.Write(System.BitConverter.GetBytes(context.blobs.Count));
            for (int i = 0; i < context.blobs.Count; i++)
            {
                var bs = context.blobs[i];
                context.writer.Write(System.BitConverter.GetBytes(bs.Length));
                context.writer.Write(bs);
            }

            context.writer.Close();
        }

        static void Collect(Context context, TypeDefinition t)
        {
            foreach (var method in t.Methods)
            {
                Collect(context, method);
            }

            foreach (var nt in t.NestedTypes)
            {
                Collect(context, nt);
            }
        }

        static void WriteMemberReference(Context context, MemberReference m)
        {
            string sNameSpace = m.DeclaringType.Namespace;
            string sTypeName = m.DeclaringType.Name;
            var t = m.DeclaringType.DeclaringType;
            while (t != null)
            {
                sTypeName = t.Name + "/" + sTypeName;
                sNameSpace = t.Namespace;
                t = t.DeclaringType;
            }

            long Namespace = context.strings.Add(sNameSpace);
            long TypeName = context.strings.Add(sTypeName);

            long Name = context.strings.Add(m.Name);

            long l = Namespace << 48 | TypeName << 32 | Name;

            var bs = System.BitConverter.GetBytes(l);

            context.writer.Write(bs);
        }

        static void WriteOperand(Context context, long value)
        {
            // context.writer.Write(System.BitConverter.GetBytes;
            context.writer.Write(System.BitConverter.GetBytes(value));
        }

        static void WriteOperand(Context context, double value)
        {
            // context.writer.Write(System.BitConverter.GetBytes;
            context.writer.Write(System.BitConverter.GetBytes(value));
        }

        static void Collect(Context context, MethodDefinition m)
        {
            context.writer.Write((byte)TableType.METHOD); // byte
            WriteMemberReference(context, m);

            int argCount = m.GenericParameters.Count + m.Parameters.Count + (m.HasThis ? 1 : 0);
            int retcount = (m.ReturnType.FullName == "System.Void") ? 0 : 1;

            context.writer.Write(System.BitConverter.GetBytes((argCount << 1) | retcount));


            Dictionary<Instruction, int> InstructionsMap = new Dictionary<Instruction, int>();
            for (int i = 0; i < m.Body.Instructions.Count; i++)
            {
                InstructionsMap[m.Body.Instructions[i]] = i;
            }

            context.writer.Write(System.BitConverter.GetBytes(m.Body.Instructions.Count)); // int32 

            for (int i = 0; i < m.Body.Instructions.Count; i++)
            {

                context.writer.Write((byte)m.Body.Instructions[i].OpCode.Code);

                var instruction = m.Body.Instructions[i];
                if (instruction.Operand == null)
                {
                    WriteOperand(context, 0);
                }
                else if (instruction.Operand is double)
                {
                    WriteOperand(context, (double)instruction.Operand);
                }
                else if (instruction.Operand is float)
                {
                    WriteOperand(context, (float)instruction.Operand);
                }
                else if (instruction.Operand is long)
                {
                    WriteOperand(context, (long)instruction.Operand);
                }
                else if (instruction.Operand is int)
                {
                    WriteOperand(context, (int)instruction.Operand);
                }
                else if (instruction.Operand is byte)
                {
                    WriteOperand(context, (byte)instruction.Operand);
                }
                else if (instruction.Operand is sbyte)
                {
                    WriteOperand(context, (sbyte)instruction.Operand);
                }
                else if (instruction.Operand is string)
                {
                    WriteOperand(context, context.strings.Add((string)instruction.Operand));
                }
                else if (instruction.Operand is Instruction)
                {
                    WriteOperand(context, InstructionsMap[(Instruction)instruction.Operand]);
                }
                else if (instruction.Operand is Instruction[])
                {
                    var Instructions = (instruction.Operand as Instruction[]);

                    // System.BitConverter
                    System.IO.MemoryStream memoryStream = new System.IO.MemoryStream();
                    for (int j = 0; j < Instructions.Length; j++)
                    {
                        var bs = System.BitConverter.GetBytes(InstructionsMap[Instructions[j]]);
                        memoryStream.Write(bs, 0, bs.Length);
                    }

                    context.blobs.Add(memoryStream.ToArray());

                    WriteOperand(context, context.blobs.Count);
                }
                else if (instruction.Operand is MemberReference)
                {
                    WriteMemberReference(context, instruction.Operand as MemberReference);
                }
                else if (instruction.Operand is VariableReference)
                {
                    WriteOperand(context, (instruction.Operand as VariableReference).Index);
                }
                else if (instruction.Operand != null)
                {
                    throw new Exception("unknown type: " + instruction.Operand.GetType().FullName);
                }
            }
        }
    }
}
