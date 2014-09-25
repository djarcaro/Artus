#!/usr/bin/env python
# -*- coding: utf-8 -*-

import logging
import Artus.Utility.logger as logger
log = logging.getLogger(__name__)

import argparse
import fnmatch
import os
import subprocess


def list_of_files(prefix, src, recursive):
	def recursive_ls(prefix, src_checked, src_to_check, recursive_mode=False):
		src_to_check_splitted = filter(lambda item: item != "", src_to_check.split("/"))
		current_src_to_check = os.path.join(src_checked, src_to_check_splitted[0])
		next_src_to_check = os.path.join(*src_to_check_splitted[1:]) if len(src_to_check_splitted) > 1 else ""
		
		# retrieve files/dirs in next level and match with src
		ls_command = "ls -1 -I " + src_checked if prefix == "" else "lcg-ls"
		next_srcs_checked = subprocess.Popen((ls_command + " " + prefix + src_checked).split(),
		                                     stdout=subprocess.PIPE).communicate()[0].split("\n")
		next_srcs_checked = [next_src_checked if next_src_checked.startswith("/") else os.path.join(src_checked, next_src_checked) for next_src_checked in next_srcs_checked]
		next_srcs_checked = [next_src_checked[:-1] if next_src_checked.endswith("/") else next_src_checked for next_src_checked in next_srcs_checked]
		next_srcs_checked = filter(lambda result: fnmatch.fnmatch(result, current_src_to_check) or fnmatch.fnmatch(os.path.join(src_checked, result), current_src_to_check), next_srcs_checked)
		
		# exit recursion
		if len(src_to_check_splitted) == 1:
			if recursive_mode:
				next_src_to_check = "*"
				if len(next_srcs_checked) == 0 or (len(next_srcs_checked) == 1 and fnmatch.fnmatch(next_srcs_checked[0], src_checked)):
					return [src_checked]
			else:
				return next_srcs_checked
		
		# recursion for next level
		src_files = []
		for next_src_checked in next_srcs_checked:
			src_files.extend(recursive_ls(prefix, next_src_checked, next_src_to_check, recursive_mode))
		return src_files
	
	files_before_recursion = recursive_ls(prefix, "/", src, recursive_mode=False)
	
	files_after_recursion = []
	if recursive:
		for file_before_recursion in files_before_recursion:
			files_after_recursion.append(recursive_ls(prefix, file_before_recursion, "*", recursive_mode=True))
	
	return files_before_recursion, files_after_recursion


def main():
	
	parser = argparse.ArgumentParser(description="Tools simplifying dCache usage.", parents=[logger.loggingParser])
	
	parser.add_argument("-c", "--command", required=True,
	                    help="Command, e.g. lcg-cp, lcg-del or ddcp.")
	parser.add_argument("-r", "--recursive", default=False, action="store_true",
	                    help="Follow recursively into all specified directories.")
	parser.add_argument("-a", "--args", help="Arguments (can be left empty).")
	parser.add_argument("-s", "--src", help="Source.", required=True)
	parser.add_argument("--src-prefix", default="",
	                    help="Source prefix. \"gridka\" and \"desy\" are replaced by their dCache locations. \"\" means local path.")
	#parser.add_argument("-d", "--dst", help="Destination (can be left empty).")
	#parser.add_argument("--dst-prefix", default="",
	#                     help="Destination prefix. \"gridka\" and \"desy\" are replaced by their dCache locations. \"\" means local path.")
	
	args = parser.parse_args()
	logger.initLogger(args)
	
	prefix_replacements = {
		"gridka" : "srm://dgridsrm-fzk.gridka.de:8443/srm/managerv2?SFN=",
		"desy" : "srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=",
	}
	for replacement_from, replacement_to in prefix_replacements.items():
		if args.src_prefix == replacement_from:
			args.src_prefix = replacement_to
		#if args.dst_prefix == replacement_from:
		#	args.dst_prefix = replacement_to
	
	src_files, src_files_recursive = list_of_files(args.src_prefix, args.src, args.recursive)
	
	for src_file in src_files:
		logger.subprocessCall((args.command + " " + (args.args if args.args else "") + " " + args.src_prefix + src_file).split())

if __name__ == "__main__":
	main()

