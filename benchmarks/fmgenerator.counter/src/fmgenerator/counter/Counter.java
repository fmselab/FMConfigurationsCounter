package fmgenerator.counter;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Path;
import java.util.List;

import org.prop4j.FMToBDD;

import de.ovgu.featureide.fm.core.base.IFeatureModel;
import de.ovgu.featureide.fm.core.base.impl.CoreFactoryWorkspaceLoader;
import de.ovgu.featureide.fm.core.base.impl.DefaultFeatureModelFactory;
import de.ovgu.featureide.fm.core.base.impl.FMFactoryManager;
import de.ovgu.featureide.fm.core.base.impl.FMFormatManager;
import de.ovgu.featureide.fm.core.base.impl.MultiFeatureModelFactory;
import de.ovgu.featureide.fm.core.editing.NodeCreator;
import de.ovgu.featureide.fm.core.io.manager.FeatureModelManager;
import de.ovgu.featureide.fm.core.io.xml.XmlFeatureModelFormat;
import net.sf.javabdd.BDD;
import picocli.CommandLine;
import picocli.CommandLine.Command;
import picocli.CommandLine.Option;

@Command(name = "BDDCounterFM", description = "Counts the number of valid products in a FM by using BDDs")
public class Counter implements Runnable {

	@Option(names = { "-m" })
	private String modelPath;

	@Option(names = { "-o" })
	private String outputFile;

	@SuppressWarnings("deprecation")
	public static void main(String[] args) {
		Counter runnable = new Counter();
		CommandLine.run(runnable, args);
	}

	@Override
	public void run() {
		// Install required extensions
		FMFactoryManager.getInstance().addExtension(DefaultFeatureModelFactory.getInstance());
		FMFactoryManager.getInstance().addExtension(MultiFeatureModelFactory.getInstance());
		FMFormatManager.getInstance().addExtension(new XmlFeatureModelFormat());
		FMFactoryManager.getInstance().setWorkspaceLoader(new CoreFactoryWorkspaceLoader());
		IFeatureModel fm = FeatureModelManager.load(Path.of(modelPath));

		// Extract the list of features
		List<String> featureList = fm.getFeatures().stream().map(t -> t.getName()).toList();

		// BDD Builder. It must be used for creating all BDDs in order to maintain the
		// same origin structure
		FMToBDD bddBuilder = new FMToBDD(featureList);

		long initialTime = System.currentTimeMillis();

		// Convert the FM into its corresponding BDD
		BDD bddNew = bddBuilder.nodeToBDD(NodeCreator.createNodes(fm));

		// Write to the output file
		FileWriter writer;
		try {
			writer = new FileWriter(new File(outputFile));
			double count = bddNew.satCount();

			writer.append(modelPath + ";" + count + ";" + (System.currentTimeMillis() - initialTime) + ";" + bddBuilder.getMaxNodes());

			writer.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
