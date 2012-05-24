package org.dynamo;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.content.Context;
import android.content.res.AssetManager;

public class ResourceManager {
	public static String RESOURCE_PATH = "GameResources";

	// Copies the game assets to /data/data/<appidentifier>/RESOURCE_PATH/ replacing existing files
	// You'll generally only want to call this after installation & updates
	private static boolean copyResources(String path) throws IOException
	{
		Context context = DynamoApp.globalContext();
		AssetManager assetManager = context.getAssets();
		String[] assets = context.getAssets().list(path);
		File destDir = new File(context.getFilesDir(), path);
		destDir.mkdirs();

		byte[] buffer = new byte[1024];
		int length;
		for (String asset : assets) {
			if(assetManager.list(path+"/"+asset).length == 0) {
				InputStream inStream = assetManager.open(path+"/"+asset);
				File outFile = new File(destDir.getAbsolutePath(), asset);
				FileOutputStream outStream = new FileOutputStream(outFile);
				
				while ((length = inStream.read(buffer)) > 0) {
					outStream.write(buffer, 0, length);
				}
				// Close the streams
				outStream.flush();
				outStream.close();
				inStream.close();
			} else
				copyResources(path+"/"+asset);
		}
		return true;
	}
	public static boolean copyResources() throws IOException { return copyResources(RESOURCE_PATH); }

	public static String pathForResource(String fileName, String extension, String subDir)
	{
		assert(fileName != null);
		String dirPath = RESOURCE_PATH;
		if(subDir != null)
			dirPath += "/"+subDir;

		String ret = new File(DynamoApp.globalContext().getFilesDir(), dirPath).getAbsolutePath();
		ret += "/"+fileName;
		if(extension != null)
			ret += "."+extension;

		return ret;
	}
	public static String pathForResource(String fileName, String extension) { return pathForResource(fileName, extension, null); }
	public static String pathForResource(String fileName) { return pathForResource(fileName, null, null); }

	public static String resourceDirPath(String path)
	{
		File destDir = new File(DynamoApp.globalContext().getFilesDir(), path);
		destDir.mkdirs();
		return destDir.getAbsolutePath();
	}
	public static String resourceDirPath() { return resourceDirPath(RESOURCE_PATH); }

}
