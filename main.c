#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Node structure for Linked List representing polynomial terms
typedef struct Node {
    int coefficient;           // Coefficient of the term
    int exponent;              // Exponent of the term
    struct Node* next;         // Pointer to the next term
} Node;

// Stack Node structure for storing Linked Lists (polynomials)
typedef struct StackNode {
    Node* polynomial;          // Linked List representing a polynomial
    struct StackNode* next;    // Pointer to the next StackNode
} StackNode;

// Stack structure to hold polynomials
typedef struct Stack {
    StackNode* top;            // Pointer to the top of the stack
} Stack;

// Function to create a new Node for a polynomial term
Node* createNode(int coefficient, int exponent) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->coefficient = coefficient;
    newNode->exponent = exponent;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a term into the Linked List (polynomial)
void insertTerm(Node** poly, int coefficient, int exponent) {
    if (coefficient == 0) return; // Skip zero coefficient terms

    Node* newNode = createNode(coefficient, exponent);

    // Insert at the beginning if the list is empty or the term has the highest exponent
    if (*poly == NULL || (*poly)->exponent < exponent) {
        newNode->next = *poly;
        *poly = newNode;
    } else {
        Node* temp = *poly;
        while (temp->next != NULL && temp->next->exponent > exponent) {
            temp = temp->next;
        }

        if (temp->exponent == exponent) {
            temp->coefficient += coefficient; // Combine terms with the same exponent
            if (temp->coefficient == 0) {
                // Remove the term if the coefficient becomes zero
                Node* toDelete = temp->next;
                temp->next = toDelete->next;
                free(toDelete);
            }
            free(newNode);
        } else {
            newNode->next = temp->next;
            temp->next = newNode;
        }
    }
}

// Function to print a polynomial (Linked List)
void printPolynomial(Node* poly) {
    if (!poly) {
        printf("Polynomial is empty.\n");
        return;
    }

    int first = 1; // Flag to handle formatting of the first term
    while (poly) {
        if (first) {
            printf("%dx^%d", poly->coefficient, poly->exponent);
            first = 0;
        } else {
            if (poly->coefficient > 0) {
                printf(" + %dx^%d", poly->coefficient, poly->exponent);
            } else {
                printf(" - %dx^%d", -poly->coefficient, poly->exponent);
            }
        }
        poly = poly->next;
    }
    printf("\n");
}

// Function to parse terms from a string into coefficient and exponent
int parseTerm(char* term, int* coefficient, int* exponent) {
    *coefficient = 0;
    *exponent = 0;

    // Remove leading spaces
    while (isspace(*term)) term++;

    if (strchr(term, 'x') != NULL) {
        // Parsing terms with 'x', e.g., 3x^2, -x, x
        if (sscanf(term, "%dx^%d", coefficient, exponent) == 2) {
            return 1;
        } else if (sscanf(term, "x^%d", exponent) == 1) {
            *coefficient = 1;
            return 1;
        } else if (sscanf(term, "-x^%d", exponent) == 1) {
            *coefficient = -1;
            return 1;
        } else if (sscanf(term, "%dx", coefficient) == 1) {
            *exponent = 1;
            return 1;
        } else if (sscanf(term, "-x", coefficient) == 1) {
            *coefficient = -1;
            *exponent = 1;
            return 1;
        }
    } else {
        // Parsing constant terms, e.g., 4, -4
        if (sscanf(term, "%d", coefficient) == 1) {
            *exponent = 0;
            return 1;
        }
    }
    return 0; // Invalid format
}

// Function to load polynomials from a file
void loadPolynomials(Stack* stack, const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Node* poly = NULL;
        char* token = strtok(line, " ");

        while (token != NULL) {
            int coefficient = 0, exponent = 0;
            if (parseTerm(token, &coefficient, &exponent)) {
                insertTerm(&poly, coefficient, exponent);
            } else {
                printf("Invalid term format: %s\n", token);
            }
            token = strtok(NULL, " ");
        }
        // Push the parsed polynomial to the stack
        StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
        newNode->polynomial = poly;
        newNode->next = stack->top;
        stack->top = newNode;
    }

    fclose(file);
    printf("Polynomials loaded successfully from file.\n");
}

// Function to add two polynomials
Node* addPolynomials(Node* poly1, Node* poly2) {
    Node* result = NULL; // Resultant polynomial
    Node* temp1 = poly1;
    Node* temp2 = poly2;

    while (temp1 != NULL || temp2 != NULL) {
        if (temp1 != NULL && (temp2 == NULL || temp1->exponent > temp2->exponent)) {
            insertTerm(&result, temp1->coefficient, temp1->exponent);
            temp1 = temp1->next;
        } else if (temp2 != NULL && (temp1 == NULL || temp2->exponent > temp1->exponent)) {
            insertTerm(&result, temp2->coefficient, temp2->exponent);
            temp2 = temp2->next;
        } else {
            int sumCoefficient = temp1->coefficient + temp2->coefficient;
            insertTerm(&result, sumCoefficient, temp1->exponent);
            temp1 = temp1->next;
            temp2 = temp2->next;
        }
    }

    return result;
}

// Function to subtract two polynomials
Node* subtractPolynomials(Node* poly1, Node* poly2) {
    Node* result = NULL;
    Node* temp1 = poly1;
    Node* temp2 = poly2;

    while (temp1 != NULL || temp2 != NULL) {
        if (temp1 != NULL && (temp2 == NULL || temp1->exponent > temp2->exponent)) {
            insertTerm(&result, temp1->coefficient, temp1->exponent);
            temp1 = temp1->next;
        } else if (temp2 != NULL && (temp1 == NULL || temp2->exponent > temp1->exponent)) {
            insertTerm(&result, -temp2->coefficient, temp2->exponent);
            temp2 = temp2->next;
        } else {
            int diffCoefficient = temp1->coefficient - temp2->coefficient;
            insertTerm(&result, diffCoefficient, temp1->exponent);
            temp1 = temp1->next;
            temp2 = temp2->next;
        }
    }

    return result;
}

// Function to multiply two polynomials
Node* multiplyPolynomials(Node* poly1, Node* poly2) {
    Node* result = NULL;

    // Traverse each term in poly1 and multiply with each term in poly2
    for (Node* temp1 = poly1; temp1 != NULL; temp1 = temp1->next) {
        for (Node* temp2 = poly2; temp2 != NULL; temp2 = temp2->next) {
            int coeff = temp1->coefficient * temp2->coefficient;
            int exp = temp1->exponent + temp2->exponent;
            insertTerm(&result, coeff, exp); // Insert the resulting term into the polynomial
        }
    }

    return result;
}

// Function to save a polynomial to a file
void savePolynomialToFile(Node* poly, const char* filepath) {
    if (!poly) {
        printf("No polynomial to save.\n");
        return;
    }

    FILE* file = fopen(filepath, "w");
    if (!file) {
        printf("Error opening file for writing.\n");
        return;
    }

    while (poly) {
        fprintf(file, "%dx^%d", poly->coefficient, poly->exponent);
        poly = poly->next;
        if (poly) fprintf(file, " ");  // Separate terms with spaces
    }

    fclose(file);
    printf("Polynomial saved to file %s.\n", filepath);
}

// Function to print all polynomials in the stack
void printStack(Stack* stack) {
    StackNode* current = stack->top;
    printf("Stack contents:\n");
    while (current != NULL) {
        printPolynomial(current->polynomial);  // Print each polynomial in the stack
        current = current->next;
    }
}

int main() {
    Stack stack = { NULL };  // Initialize stack with NULL (empty)
    int choice;
    char filename[100];

    do {
        printf("Menu:\n");
        printf("1. Load polynomials from file\n");
        printf("2. Print loaded polynomials\n");
        printf("3. Add two polynomials\n");
        printf("4. Subtract two polynomials\n");
        printf("5. Multiply two polynomials\n");
        printf("6. Perform operation on stack\n");
        printf("7. Save polynomial to file\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter filename to load: ");//Load polynomials from file
                scanf("%s", filename);
                loadPolynomials(&stack, filename);
                break;
            case 2:
                printStack(&stack);//Print loaded polynomials
                break;
            case 3: {
                if (stack.top != NULL && stack.top->next != NULL) {
                    Node* result = addPolynomials(stack.top->polynomial, stack.top->next->polynomial);
                    printf("Result of addition: ");
                    printPolynomial(result);
                    // Push the result onto the stack
                    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
                    newNode->polynomial = result;
                    newNode->next = stack.top;
                    stack.top = newNode;
                } else {
                    printf("Not enough polynomials in the stack.\n");
                }
                break;
            }
            case 4: {
                if (stack.top != NULL && stack.top->next != NULL) {
                    Node* result = subtractPolynomials(stack.top->polynomial, stack.top->next->polynomial);
                    printf("Result of subtraction: ");
                    printPolynomial(result);
                    // Push the result onto the stack
                    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
                    newNode->polynomial = result;
                    newNode->next = stack.top;
                    stack.top = newNode;
                } else {
                    printf("Not enough polynomials in the stack.\n");
                }
                break;
            }
            case 5: {
                if (stack.top != NULL && stack.top->next != NULL) {
                    Node* result = multiplyPolynomials(stack.top->polynomial, stack.top->next->polynomial);
                    printf("Result of multiplication: ");
                    printPolynomial(result);
                    // Push the result onto the stack
                    StackNode* newNode = (StackNode*)malloc(sizeof(StackNode));
                    newNode->polynomial = result;
                    newNode->next = stack.top;
                    stack.top = newNode;
                } else {
                    printf("Not enough polynomials in the stack.\n");
                }
                break;
            }
            case 6:
                // Perform stack operations like pop, push, etc. (You can define it later)
                break;
            case 7: // save the result to result.txt
                if (stack.top != NULL) {
                    savePolynomialToFile(stack.top->polynomial, "result.txt");
                } else {
                    printf("No polynomials to save.\n");
                }
                break;
            case 8:
                printf("Exiting program.\n");//exit
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}
