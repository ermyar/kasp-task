import tempfile
import hashlib
import pathlib
import os
import subprocess
import sys
import random

def run_test(fn):
	with tempfile.TemporaryDirectory() as tmpdir:
		extra_args, checker = fn(tmpdir)
		args = [sys.argv[1]] + extra_args

		with subprocess.Popen(args, stderr=subprocess.PIPE, stdout=subprocess.PIPE, encoding="utf-8") as proc:
			stdout, stderr = proc.communicate()

		checker(proc.returncode, stdout, stderr)



def get_answer(options):
	ans = []

	if len(options) == 0:
		options.append(".")

	for dirpath, _, filenames in os.walk(options[0]):
		for f in filenames:
			fname = os.path.join(dirpath, f)

			with open(fname, "rb") as bin:
				bytes = bin.read()
				hash = hashlib.sha256(bytes).hexdigest()
				ans.append(f"{fname[len(options[0])+1:]}  :  {hash}")
				# print(ans[-1])

	return ans


def check_ans(tmpdir, stdout):
	res = get_answer(tmpdir)
	res.sort()

	lst = stdout.split('\n')
	if len(lst) > 0 and lst[-1] == '':
		lst = lst[:-1]
	lst.sort()

	assert lst == res,  f"\nexpected:\n{res}\n\nactual:\n{lst}"


def check_no_flag(tmptp):

	def checker(returncode, stdout, stderr):
		assert returncode == 0, f"{stderr}\n{stdout}"
		check_ans([], stdout)

	return [], checker


def check_small_dir(tmpdir):

	for i in range(10):
		fname = os.path.join(tmpdir, f"file_{i}")

		with open(fname, "wb") as ff:
			ln = random.randint(100, 1000)
			ff.write(os.urandom(ln))


	def checker(returncode, stdout, stderr):
		assert returncode == 0, f"{stderr}\n{stdout}"
		check_ans([tmpdir], stdout)

	return [tmpdir], checker


def check_big_dir(tmpdir):
	lastdir = tmpdir

	for i in range(200):
		fname = os.path.join(lastdir, f"file{i}")
		lastdir = os.path.join(lastdir, f"dir{i}")
		pathlib.Path(lastdir).mkdir()

		with open(fname, "wb") as ff:
			ln = random.randint(2000, 10000)
			ff.write(os.urandom(ln))


	def checker(returncode, stdout, stderr):
		assert returncode == 0, f"{stderr}\n{stdout}"
		check_ans([tmpdir], stdout)

	return [tmpdir], checker


def check_failture_not_dir(tmpdir):
	nfile = os.path.join(tmpdir, "newfile")

	pathlib.Path(nfile).touch()

	def checker(returncode, stdout, stderr):
		assert returncode != 0, f"{stderr}\n{stdout}"
		assert "DIR_PATH not a directory!" in stderr, stdout

	return [nfile], checker

run_test(check_no_flag)
run_test(check_small_dir)
run_test(check_big_dir)
run_test(check_failture_not_dir)
