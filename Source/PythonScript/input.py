

def get_integer(string, minimum, maximum, error_message='Invalid option', check_expr=None):
    '''
    Funciton Name: get_integer
    Description: This function is used to get a valid integer input from the user.
    :param string: The message for getting input
    :type string: str
    :param minimum: minimum valid value of the input
    :type minimum: int
    :param maximum: maximum valid value of the input
    :type maximum: int
    :param error_message: Error message which needs to be shown when input is not valid
    :type error_message: str
    :param check_expr: expression which needs to be checked, by default its none.
    :type check_expr: Any
    :return: valid integer from the user
    :rtype: int
    '''

    integer = -1
    while True:
        try:
            integer = int(input(string))
            if maximum >= integer >= minimum:
                if check_expr is None or check_expr(integer):
                    break
                continue
            else:
                print(error_message)
                continue
        except ValueError:
            print(error_message)
            continue

    return integer


def get_yes_or_no():
    '''
    Function Name: get_yes_or_no
    Description: This function is to get yes or no input from the user.
    :return: True or False
    :rtype: bool
    '''

    while True:
        try:
            character = input("Enter y/Y to Continue or n/N to dis-Continue:")
            if character.lower() == 'y':
                return True
            elif character.lower() == 'n':
                return False
            else:
                print("Invalid input,Please")
                continue
        except ValueError:
            print("Invalid input,Please")
            continue
