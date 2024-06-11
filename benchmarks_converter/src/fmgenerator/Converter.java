package fmgenerator;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Paths;

import de.ovgu.featureide.fm.core.base.IFeatureModel;
import de.ovgu.featureide.fm.core.base.impl.DefaultFeatureModelFactory;
import de.ovgu.featureide.fm.core.init.FMCoreLibrary;
import de.ovgu.featureide.fm.core.io.manager.SimpleFileHandler;
import de.ovgu.featureide.fm.core.io.sxfm.SXFMFormat;
import de.ovgu.featureide.fm.core.io.xml.XmlFeatureModelFormat;

public class Converter {

	public static String origin = "..\\SXFM";
	public static String destination = "..\\FeatureIDE\\";

	public static void main(String[] args) throws Exception {
		FMCoreLibrary.getInstance().install();
		Files.walk(Paths.get(origin)).filter(x -> x.getFileName().toString().endsWith(".xml")).forEach(x -> {
			IFeatureModel fm_original = DefaultFeatureModelFactory.getInstance().create();
			SXFMFormat format = new SXFMFormat();
			SimpleFileHandler.load(x, fm_original, format);
			// save with the same name in the other directory (same format sxfm) using the
			// featureidewriter
			String newPath = destination + x.getFileName().toString();
			File newFile = new File(newPath);
			if (SimpleFileHandler.save(newFile.toPath(), fm_original, new XmlFeatureModelFormat()).containsError()) {
				newFile.delete();
			}
		});

	}
}
