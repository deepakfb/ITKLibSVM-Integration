#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkImageToImageFilter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "svm.h"
#include "ImageReaderWriter.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void print_null(const char *s) {}

void do_cross_validation();
void exit_input_error(int line_num);
int svm_train_main(float* labels, float** featurevectors, int numberofvectors);


struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;
int cross_validation;
int nr_fold;

static char *line = NULL;
static int max_line_len;
int featurelength = 3;

// Arguments: "training image" "label image" "test image" "output image"
int main( int argc, char **argv ) {

	if(argc<5)
	{
		std::cerr << "Incorrect Arguments! Please supply following four arguments  in the same order (1) training image (2) label image (3) test image (4)  output image" << std::endl;
		getchar();
		return EXIT_FAILURE;		
	}

  try
  {
		float* labels=NULL; float** featurevectors=NULL;int *width=(int*)malloc(sizeof(int)), *height=(int*)malloc(sizeof(int)), *numfeatures=(int*)malloc(sizeof(int));

		if(!readImageandLabels(argv[1], argv[2], &labels, &featurevectors, width, height, numfeatures))
		{
			std::cerr << "Error in reading input or label image. Please ensure label image has all three color planes." << std::endl;
			free(width);
			free(height);
			free(numfeatures);
			getchar();
			return EXIT_FAILURE;					
		}		

		// call svm train function
	  if(svm_train_main(labels, featurevectors, *numfeatures)==0)
	  {
		  // call svm predict function
		  	int svm_type=svm_get_svm_type(model);
			int nr_class=svm_get_nr_class(model);

			free(featurevectors);

			// Test
			float** featurevectors=NULL; int *testimagewidth=(int*)malloc(sizeof(int)), *testimageheight = (int*)malloc(sizeof(int)); float* predictedlabels;
			if(!readImage(argv[3], &featurevectors, testimagewidth, testimageheight))
			{
				std::cerr << "Error in reading test image." << std::endl;
		
				free(width);
				free(height);
				free(numfeatures);
				free(testimagewidth);
				free(testimageheight);
				free(labels);
				free(model);

				getchar();
				return EXIT_FAILURE;					
			}

			predictedlabels =(float*) malloc((*testimagewidth)*(*testimageheight)*sizeof(float));

			struct svm_node *x;
			x = (struct svm_node *) malloc((featurelength+1)*sizeof(struct svm_node));
			for(int i=0;i<(*testimagewidth)*(*testimageheight);i++)
			{				
				for(int j=0;j<featurelength;j++)
				{
				x[j].index = j+1;
				x[j].value = featurevectors[i][j];
				}
				x[featurelength].index = -1;
				predictedlabels[i] =  svm_predict(model, x);								
			}
			free(x);
		 
		 if(!writeLabels(predictedlabels, *testimagewidth, *testimageheight, argv[4]))
		 {
				std::cerr << "Error in writing output image." << std::endl;
				free(width);
				free(height);
				free(numfeatures);
				free(testimagewidth);
				free(testimageheight);
				free(predictedlabels);
				free(featurevectors);
				free(labels);
				free(model);

				getchar();
				return EXIT_FAILURE;					

		 }

		free(width);
		free(height);
		free(numfeatures);
		free(testimagewidth);
		free(testimageheight);
		free(predictedlabels);				
		free(labels);
		free(featurevectors);
		free(model);
	  }
	  else
	  {
		std::cerr << "Error in training SVM classifier." << std::endl;
		free(featurevectors);
		free(width);
		free(height);
		free(labels);
		free(numfeatures);
		
		getchar();
		return EXIT_FAILURE;					
	  }
  }
  catch(itk::ExceptionObject & err)
  {
	std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
  }
  return 0;
}



int svm_train_main(float labels[], float *featurevectors[], int numberofvectors)
{
	// default values
	param.svm_type = 0;
	param.kernel_type = 0;// RBF;
	param.degree = 3;
	param.gamma = 1/3;
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 0.1;//1e-3;
	param.p = 0.1;
	param.shrinking = 0;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;
	
	const char *error_msg;


	prob.l = numberofvectors; // number of features
	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);

	x_space = Malloc(struct svm_node,prob.l*featurelength + prob.l); // number of features*feature length + number of features for -1 index entry
	int j=0;

	for(int i=0;i<prob.l;i++)
	{
		prob.x[i] = &x_space[j];
		double label = labels[i];
		prob.y[i] = label;
		for(int k=0;k<featurelength;k++)
		{
			x_space[j].index = k+1;
			x_space[j].value = featurevectors[i][k];
			++j;
		}
		x_space[j].index = -1;
	}

	param.gamma = 1.0/(float)(featurelength);

	error_msg = svm_check_parameter(&prob,&param);

	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

	if(cross_validation)
	{
		do_cross_validation();
	}
	else
	{
		model = svm_train(&prob,&param);
	}
	return 0;
}


static char* readline(FILE *input)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}



void exit_with_help()
{
	printf(
	"Usage: svm-train [options] training_set_file [model_file]\n"
	"options:\n"
	"-s svm_type : set type of SVM (default 0)\n"
	"	0 -- C-SVC		(multi-class classification)\n"
	"	1 -- nu-SVC		(multi-class classification)\n"
	"	2 -- one-class SVM\n"
	"	3 -- epsilon-SVR	(regression)\n"
	"	4 -- nu-SVR		(regression)\n"
	"-t kernel_type : set type of kernel function (default 2)\n"
	"	0 -- linear: u'*v\n"
	"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
	"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
	"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
	"	4 -- precomputed kernel (kernel values in training_set_file)\n"
	"-d degree : set degree in kernel function (default 3)\n"
	"-g gamma : set gamma in kernel function (default 1/num_features)\n"
	"-r coef0 : set coef0 in kernel function (default 0)\n"
	"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
	"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
	"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
	"-m cachesize : set cache memory size in MB (default 100)\n"
	"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
	"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
	"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
	"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
	"-v n: n-fold cross validation mode\n"
	"-q : quiet mode (no outputs)\n"
	);
	exit(1);
}

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}





void do_cross_validation()
{
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double,prob.l);

	svm_cross_validation(&prob,&param,nr_fold,target);
	if(param.svm_type == EPSILON_SVR ||
	   param.svm_type == NU_SVR)
	{
		for(i=0;i<prob.l;i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n",total_error/prob.l);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
			((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy))
			);
	}
	else
	{
		for(i=0;i<prob.l;i++)
			if(target[i] == prob.y[i])
				++total_correct;
		printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
	}
	free(target);
}


