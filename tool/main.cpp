#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"

#include "dot.cpp"

#include <iostream>

using namespace llvm;

/// A category for the options specified for this tool.
static cl::OptionCategory AddConstCategory("addconst pass options");

/// A required argument that specifies the module that will be transformed.
static cl::opt<std::string> InputModule(cl::Positional,
                                        cl::desc("<Input module>"),
                                        cl::value_desc("bitcode filename"),
                                        cl::init(""), cl::Required,
                                        cl::cat(AddConstCategory));

/// An optional argument that specifies the name of the output file.
static cl::opt<std::string> OutputModule("o", cl::Positional,
                                         cl::desc("<Output module>"),
                                         cl::value_desc("dot file name"),
                                         cl::init("out.dot"),
                                         cl::cat(AddConstCategory));

std::string genOpStr(Value *operand) {
  if (isa<Function>(operand)) {
    return "@" + operand->getName().str();
  }

  if (isa<BasicBlock>(operand)) {
    return operand->getName().str();
  }

  if (ConstantInt *CI = dyn_cast<ConstantInt>(operand)) {
    return std::to_string(CI->getZExtValue());
  }

  if (ConstantExpr *CE = dyn_cast<ConstantExpr>(operand)) {
    std::string rep = "";

    for (int j = 0; j < CE->getNumOperands(); j++) {
      rep += genOpStr(CE->getOperand(j));
    }

    return rep;
  }

  if (operand->hasName())
    return "%" + operand->getName().str();

  return "";
}

std::string getInstructionStr(Instruction &I) {
  string result;

  if (!I.getType()->isVoidTy()) {
    result += "%" + I.getName().str() + " = ";
  }

  result += I.getOpcodeName();

  if (PHINode *PN = dyn_cast<PHINode>(&I)) {

    for (int i = 0; i < PN->getNumIncomingValues(); i++) {
      result += std::string(i ? ", " : "");
      result += " [" + genOpStr(PN->getIncomingValue(i));
      result += ", " + genOpStr(PN->getIncomingBlock(i)) + " ]";
    }

    return result + "\\l";
  }
	
  if (I.getOpcode() == Instruction::Call) {
    result += " " + genOpStr(I.getOperand(I.getNumOperands() - 1));
    result += "(";

    for (int i = 0; i < I.getNumOperands() - 1; i++) {
      result += genOpStr(I.getOperand(i));
      if (i < I.getNumOperands() - 2) {
        result += ", ";
      }
    }

    result += ")";

    result += "\\l";

    return result;
  }

  // for (Use &operand : I.operands()) {
  for (int i = I.getNumOperands() - 1; i >= 0; i--) {
    result += " " + genOpStr(I.getOperand(i));
  }

  return result + "\\l";
}

std::string getBasicBlockStr(BasicBlock &BB) {
  std::string bbStr = BB.getName().str() + ":\\l\\l";

  for (Instruction &I : BB)
    bbStr += getInstructionStr(I);

  return bbStr;
}

void genBlocksAndInstructionsNames(Function &Fn) {
  int bb_counter = 0;
  int in_counter = 1;

  for (BasicBlock &BB : Fn) {
    BB.setName("BB" + to_string(bb_counter++));

    for (Instruction &I : BB) {
      if (!I.hasName() && !I.getType()->isVoidTy()) {
        I.setName(to_string(in_counter++));
			}
		}
  }
}

std::string compileFunctionToDot(Function &Fn) {

  genBlocksAndInstructionsNames(Fn);

  std::string title = Fn.getName().str();

  dot::Graph graph(title);

  for (BasicBlock &BB : Fn)
    graph.node(BB.getName().str(), getBasicBlockStr(BB));

  for (BasicBlock &BB : Fn)
    for (BasicBlock *BBSucc : successors(&BB))
      graph.edge(BB.getName().str(), BBSucc->getName().str());

  return graph.genDot();
}

struct DotPass : public PassInfoMixin<DotPass> {

  PreservedAnalyses run(Function &Fn, FunctionAnalysisManager &AM) {

    std::cout << compileFunctionToDot(Fn) << "\n";

    return PreservedAnalyses::all();
  }
};

int main(int Argc, char **Argv) {
  // Hide all options apart from the ones specific to this tool.
  cl::HideUnrelatedOptions(AddConstCategory);

  // Parse the command-line options that should be passed to the invariant
  // pass.
  cl::ParseCommandLineOptions(Argc, Argv,
                              "generate dot representation for CFG's\n");

  // Makes sure llvm_shutdown() is called (which cleans up LLVM objects)
  // http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
  llvm_shutdown_obj SDO;

  // Parse the IR file passed on the command line.
  SMDiagnostic Err;
  LLVMContext Ctx;
  std::unique_ptr<Module> M = parseIRFile(InputModule.getValue(), Err, Ctx);

  if (!M) {
    errs() << "Error reading bitcode file: " << InputModule << "\n";
    Err.print(Argv[0], errs());
    return -1;
  }

  // Create a FunctionPassManager and add the AddConstPass to it:
  FunctionPassManager FPM;
  FPM.addPass(DotPass());

  FunctionAnalysisManager FAM;
  // Register the module analyses:
  PassBuilder PB;
  PB.registerFunctionAnalyses(FAM);
  // Finally, run the passes registered with FPM.
  for (Function &F : *M) {
    FPM.run(F, FAM);
  }

  return 0;
}
