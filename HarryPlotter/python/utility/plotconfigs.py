#!/usr/bin/env python
# -*- coding: utf-8 -*-

import logging
import Artus.Utility.logger as logger
log = logging.getLogger(__name__)

import copy
import os

import ROOT
ROOT.gROOT.SetBatch(True)
ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gErrorIgnoreLevel = ROOT.kError

import Artus.HarryPlotter.utility.roottools as roottools
import Artus.Utility.tfilecontextmanager as tfilecontextmanager


class PlotConfigs(object):
	def __init__(self):
		super(PlotConfigs, self).__init__()
	
	@staticmethod
	def all_histograms(root_filename, plot_config_template=None):
		if plot_config_template is None:
			plot_config_template = {}
		
		plot_configs = []
		with tfilecontextmanager.TFileContextManager(root_filename, "READ") as root_file:
			elements = roottools.RootTools.walk_root_directory(root_file)
			for key, path in elements:
				if key.GetClassName().startswith("TH") or key.GetClassName().startswith("TProfile"):
					plot_config = copy.deepcopy(plot_config_template)
					plot_config["files"] = root_filename
					plot_config["folders"] = os.path.dirname(path)
					plot_config["x_expressions"] = os.path.basename(path)
					plot_config["title"] = path
					plot_config["x_label"] = ""
					plot_config["output_dir"] = os.path.join(os.path.splitext(root_filename)[0], os.path.dirname(path))
					plot_config["filename"] = os.path.basename(path)
					plot_configs.append(plot_config)
		return plot_configs
	
	@staticmethod
	def all_runtimes(root_filename, plot_config_template=None):
		if plot_config_template is None:
			plot_config_template = {}
		
		plot_configs = []
		with tfilecontextmanager.TFileContextManager(root_filename, "READ") as root_file:
			elements = roottools.RootTools.walk_root_directory(root_file)
			for key, path in elements:
				if path.endswith("runTime") and (key.GetClassName().startswith("TTree") or key.GetClassName().startswith("TNtuple")):
					tree = root_file.Get(path)
					for branch in [branch.GetName() for branch in tree.GetListOfBranches()]:
						plot_config = copy.deepcopy(plot_config_template)
						plot_config["files"] = root_filename
						plot_config["folders"] = [path]
						plot_config["x_expressions"] = [branch]
						plot_config["weights"] = ["({0}>=0.0)".format(branch)]
						plot_config["title"] = branch
						plot_config["x_label"] = "Runtime per Event / #mus"
						plot_config["output_dir"] = os.path.join(plot_config.get("output_dir", ""), os.path.dirname(path))
						plot_config["filename"] = branch
						plot_configs.append(plot_config)
		return plot_configs

