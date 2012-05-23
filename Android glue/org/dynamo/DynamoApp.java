package org.dynamo;

import java.io.IOException;

import android.app.Application;
import android.content.Context;

public class DynamoApp extends Application {
	private static DynamoApp sharedInstance = null;
	
	public static Context globalContext()
	{
		if(sharedInstance == null)
			throw new IllegalStateException("Application not created yet!");
		return sharedInstance.getApplicationContext();
	}

	@Override
	public void onCreate() {
		super.onCreate();
		sharedInstance = this;
		
		System.loadLibrary("dynamo");

		try {
			ResourceManager.copyResources();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
