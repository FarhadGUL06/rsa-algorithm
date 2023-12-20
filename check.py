import subprocess
import sys
import os
import re
import timeit
import pickle
import datetime

implementations = {}
solutions = ["run_serial_opt", "run_openmp", "run_pthread", "run_mpi", "run_mpi_openmp", "run_cuda_opt"]

make_clean = ['make', 'clean']
make = ['make']

def save_database():
    # Current time as name
    database_name = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    file_database = open("./stats/" + database_name + ".pkl", "wb")
    pickle.dump(implementations, file_database)
    file_database.close()

def run_c_program(input_file, exec_file):

    # Take file name from input_file and save it to file_name
    file_name = re.split("/", input_file)[-1]

    output_file = f"tests/output/{file_name}"

    args = f"ARGS=\"{file_name}\""

    # Use timeit to determine time of execution
    command = ['make', exec_file, args]

    # Test dimension for timings
    test_size = 0

    try:
        input_text = ""
        with open(input_file, 'r') as input_file_handle:
            input_text = input_file_handle.read().strip()
            # get size of the text for the graphics
            test_size = len(input_text)
        repeats = 1
        with open(input_file, 'r') as input_file_handle:
            result = timeit.timeit(lambda: subprocess.run(command, stdin=input_file_handle, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True), number=repeats)
            result = result / repeats

        with open(output_file, 'r') as output_file_handle:
            output_text = output_file_handle.read().strip()

        decriptat = re.split("Decriptat: ", output_text)[1]
        decriptat = decriptat.replace("\n", "")

        if decriptat == input_text:
            status = "OK"
        else:
            status = "FAILED"

        print(file_name + ": " + status + " -> " + str("{0:.2f}".format(result)))
        timings.update({test_size: float("{0:.3f}".format(result))})
        # Get the time taken by the RSA solution
    

    except subprocess.CalledProcessError as e:
        print(f"Error for {input_file}:")
        print(e.stderr)


def run_tests(exec_name):
    # Automatically determine the number of input files
    input_files = [f for f in os.listdir(input_folder) if os.path.isfile(os.path.join(input_folder, f))]
    num_files = len(input_files)
    # Assuming input files are named input1.txt, input2.txt, etc.
    if num_files == 0:
        print(f"No input files found in the {input_folder} folder.")
        sys.exit(1)

    # Remove last results "rm -rf ./tests/output" and recreate it
    subprocess.run(['rm', '-rf', './tests/output/*'])  

    global timings
    timings = {}

    print("For " + exec_name + ":")

    for i in range(0, num_files - 1):
        if i < 10:
            no_file = "0" + str(i)
        else:
            no_file = str(i)
        input_file = f"{input_folder}/test{no_file}.txt"
        run_c_program(input_file, exec_name)

    # Run test huge if exec_name != "run_serial_opt"
    if exec_name != "run_serial_opt":
        print("Testing huge input")
        input_file = f"{input_folder}/test_huge.txt"
        run_c_program(input_file, exec_name)
    
    subprocess.run(['rm', '-rf', './tests/output/*'])
    # Add timings to current implementation
    implementations.update({exec_name: timings})
    

def run_perf(exec_name, test_number):
    '''
    Perf commands:
    sudo perf stat -o ./perf/stat/<exec_name>.stat <exec_name> <test_name>
    sudo perf record -g -o ./perf/record/<exec_name>.data <exec_name> <test_name>
    sudo perf report -i ./perf/record/<exec_name>.data
    '''
    subprocess.run(['rm', '-rf', f'./perf/stat/{exec_name}.stat'])
    subprocess.run(['rm', '-rf', f'./perf/record/{exec_name}.data'])

    if int(test_number) < 10:
        no_file = "0" + str(test_number)
    else:
        no_file = str(test_number)
    input_file = f"test{no_file}.txt"    


    if exec_name == "run_serial_opt":
        exec_name = "serial_opt"
        final_command = f"./{exec_name} " + input_file
    elif exec_name == "run_openmp":
        exec_name = "openmp"
        final_command = f"./{exec_name} " + input_file
    elif exec_name == "run_pthread":
        exec_name = "pthread"
        final_command = f"./{exec_name} 8 " + input_file
    elif exec_name == "run_mpi":
        exec_name = "mpi"
        final_command = f"mpirun -np 8 ./{exec_name} " + input_file
    elif exec_name == "run_mpi_openmp":
        exec_name = "mpi_openmp"
        final_command = f"mpirun -np 8 ./{exec_name} " + input_file
    elif exec_name == "run_cuda_opt":
        exec_name = "cuda_opt"
        final_command = f"./{exec_name} " + input_file
    elif exec_name == "run_serial":
        exec_name = "serial"
        final_command = f"./{exec_name} " + input_file
    
    else:
        print("Invalid exec_name")
        sys.exit(1)

    print ("Running perf on " + final_command)
    # Make an array from the command
    final_command = final_command.split(" ")


    # Run perf on the test
    command = ['sudo', 'perf', 'stat', '-o', f'./perf/stat/{exec_name}.stat']
    command += final_command
    subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Run perf record on the test
    command = ['sudo', 'perf', 'record', '-g', '-o', f'./perf/record/{exec_name}.data']
    command += final_command
    subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # Run perf report on the test
    command = ['sudo', 'perf', 'report', '-i', f'./perf/record/{exec_name}.data']
    #subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    print("Run yourself perf report on the test using this command:")
    
    # Combine command in a string separated by spaces
    newcommand = ""
    for i in range(0, len(command)):
        newcommand += command[i] + " "
    
    print(newcommand)
    
    


if __name__ == "__main__":
    subprocess.run(make_clean, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.run(make, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    input_folder = "./tests/input"
    if len(sys.argv) < 2:
        print("Usage: python3 check.py <type_check> <exec_name>*")
        sys.exit(1)

    type_check = sys.argv[1]

    if type_check == "stats":
        for solution in solutions:
            run_tests(solution)
        print(implementations)
        save_database()
    else:
        exec_name = sys.argv[2]

        if type_check == "test":
            run_tests(exec_name)
        
        if type_check == "perf":
            test_number = sys.argv[3]
            run_perf(exec_name, test_number)
