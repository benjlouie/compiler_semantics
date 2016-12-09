/**
* 
* I didn't have enough time to implement the full loopUnrolling function.
* I managed to get the conditionals of the loop however I wasn't able to do anything with them
* In the readme I will describe what I would do in order to implement the rest of the unroll function
*
*/


#include "loopUnroll.h"

void unrollWhile(void) {
	
	/* Get the Nodes from the AST*/
	/* Similar to the function we wrote for classInheritance */
	auto classNodes = root->getChildren();
	
	for (Tree *tchild : classNodes)
	{
		Node *child = (Node*)tchild;
		auto classDescNodes = child->getChildren();
		string className = ((Node *)classDescNodes[0])->value;

		/* [2] contains the method nodes of the AST */
		Node *method = ((Node *)classDescNodes[2]);
		//std::cout << className << "\n";

		/* Need a list of the classes to visit */
		vector<string> inheritList;

		/* Gets each of the classes that are in the parent node */
		while (className != "Object")
		{
			inheritList.push_back(className);
			className = globalTypeList[className]; //This allows to check the list
			//std::cout << className << "\n";
		}

		/* Need to get the methods from the tree */
		//Node* methods = method->getChildren();

		for (Tree *tMethod : method->getChildren())
		{
			/* Need methods because while loops will be located inside of the methods*/
			/* Don't need variables just yet */
			Node *method = (Node*)tMethod;
			
			if (method->type == AST_FEATURE_METHOD) {

				/* Gets the children of the method */
				auto methodNodes = method->getChildren();

				/* Saves the method name to make sure it found the right method! */
				string methName = ((Node*)methodNodes[0])->value;
				Node *whileNodes = ((Node*)methodNodes[3]);
				std::cout << methName << "\n";

				/* Checks if we have a while loop in the AST */
				if ((whileNodes->type) == AST_WHILE) {
					/* Get the while loops*/
					for (Tree *tWhile : whileNodes->getChildren())
					{
						Node *whileNodes = (Node*)tWhile;
						
						/* Gets the expression and the condition of the loop */
						auto loopNodes = whileNodes->getChildren();
						
						Node *express = (Node*)loopNodes[0];
						Node *condition = (Node*)loopNodes[1];
						string exprName = ((Node*)loopNodes[0])->value;
						string condName = ((Node*)loopNodes[1])->value;
						string exprType = ((Node*)loopNodes[0])->valType;
						string condType = ((Node*)loopNodes[1])->valType;
						

	
						if (exprType != "SELF_TYPE") {
							std::cout << exprName << "\n";
							std::cout << condName << "\n";
							//std::cout << exprType << "\n";
							//std::cout << condType << "\n";


						}

						/* Check the condition of the while loop - Determines how many times a loop is unrolled */
						/* Unroll based on the condition of the while loop */
						/* Copy the expression block the number of times to unroll */

					}
				}
			}
		}
	}
}