from  tests.correctness.checker import *
import re 

class AnswerSetValidator:

    def __init__(self, path):
        self.path = path


    def valid(self, command, benchmark, testcase, xml):
        command = command.id
        if re.search(r"plain",command):
            return True
        
        output_file = xml.xpath("//stats/@output")[0]
        try:
            return check(encoding_path=self.path, instance_path=testcase[0], file_path=output_file)
        except:
            return False

    def setDirname(self, dirname):
        pass

class ExitCodeValidator:
    def __init__(self, validExitCodes=[10, 20]):
        self.validExitCodes = validExitCodes
        
    def valid(self, command, benchmark, testcase, xml):
        exit_code = int(xml.xpath("//stats/@result")[0])
        return exit_code in self.validExitCodes

    def setDirname(self, dirname):
        pass