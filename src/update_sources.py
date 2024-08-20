import subprocess

def run_cogapp_on_cmake():
    try:
        # Execute the command
        result = subprocess.run(['python', '-m', 'cogapp', '-r', 'CMakeLists.txt'], check=True, capture_output=True, text=True)

        # Print the output of the command
        print(result.stdout)
        if result.stderr:
            print("Errors:", result.stderr)

    except subprocess.CalledProcessError as e:
        print(f"Command failed with exit code {e.returncode}")
        print("Output:", e.output)
        print("Error:", e.stderr)

if __name__ == "__main__":
    run_cogapp_on_cmake()
