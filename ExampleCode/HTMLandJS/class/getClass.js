function get(){

	console.log(isense.getDatasetFieldData(106,"MLB Team Statistics 2013","Hits"));  // Project ID, Dataset Name, Field Name


	data = [5,4,8,4,5,3,3,84,5484]



	isense.postDataset(927,"Key","Dice Roll","TEST","Tyler",data);  // Project ID,Contributor Key,Title,Contributor Name,data as an array
}