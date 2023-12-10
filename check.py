import subprocess
import sys
import os
import re  

make_clean = ['make', 'clean']
make = ['make']

def run_c_program(input_file, exec_file):

    # Take file name from input_file and save it to file_name
    file_name = re.split("/", input_file)[-1]

    output_file = f"tests/output/{file_name}"

    args = f"ARGS=\"{file_name}\""


    #deleteCache = ['echo', '3', '>', '/proc/sys/vm/drop_caches']
    command = ['/usr/bin/time', '-f', '%e', 'make', exec_file, args]
    try:
        input_text = ""
        with open(input_file, 'r') as input_file_handle:
            input_text = input_file_handle.read().strip()

        with open(input_file, 'r') as input_file_handle:
            #subprocess.run(make_clean, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            #subprocess.run(['sync'])
            #subprocess.run(deleteCache, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            #subprocess.run(make, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            result = subprocess.run(command, stdin=input_file_handle, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        with open(output_file, 'r') as output_file_handle:
            output_text = output_file_handle.read().strip()

        decriptat = re.split("Decriptat: ", output_text)[1]
        decriptat = decriptat.replace("\n", "")

        if decriptat == input_text:
            status = "OK"
        else:
            status = "FAILED"

        real_time = result.stderr.strip()  # Extract real time and remove leading/trailing whitespaces

        print(file_name + ": " + status + " -> " + str(real_time))

    except subprocess.CalledProcessError as e:
        print(f"Error for {input_file}:")
        print(e.stderr)


def run_tests():
    # Automatically determine the number of input files
    input_files = [f for f in os.listdir(input_folder) if os.path.isfile(os.path.join(input_folder, f))]
    num_files = len(input_files)
    # Assuming input files are named input1.txt, input2.txt, etc.
    if num_files == 0:
        print(f"No input files found in the {input_folder} folder.")
        sys.exit(1)

    # Remove last results "rm -rf ./tests/output" and recreate it
    subprocess.run(['rm', '-rf', './tests/output'])
    subprocess.run(['mkdir', './tests/output'])

    print("For " + exec_name + ":")

    for i in range(0, num_files - 1):
        if i < 10:
            no_file = "0" + str(i)
        else:
            no_file = str(i)
        input_file = f"{input_folder}/test{no_file}.txt"
        run_c_program(input_file, exec_name)

    print("Testing huge input")
    input_file = f"{input_folder}/test_huge.txt"
    run_c_program(input_file, exec_name)

def run_perf():
    # Automatically determine the number of input files
    input_files = [f for f in os.listdir(input_folder) if os.path.isfile(os.path.join(input_folder, f))]
    num_files = len(input_files)
    # Assuming input files are named input1.txt, input2.txt, etc.
    if num_files == 0:
        print(f"No input files found in the {input_folder} folder.")
        sys.exit(1)

    print("For " + exec_name + ":")

    for i in range(0, num_files - 1):
        if i < 10:
            no_file = "0" + str(i)
        else:
            no_file = str(i)
        input_file = f"{input_folder}/test{no_file}.txt"
        run_c_program(input_file, exec_name)

    print("Testing huge input")
    input_file = f"{input_folder}/test_huge.txt"
    run_c_program(input_file, exec_name)

if __name__ == "__main__":
    subprocess.run(make_clean, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.run(make, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    input_folder = "./tests/input"
    if len(sys.argv) < 3:
        print("Usage: python3 check.py <type_check> <exec_name>")
        sys.exit(1)

    type_check = sys.argv[1]
    exec_name = sys.argv[2]
    
    if type_check == "test":
        run_tests()
    
    if type_check == "perf":
        run_perf()

