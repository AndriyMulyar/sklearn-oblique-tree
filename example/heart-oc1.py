import pandas as pd 
import numpy as np 
import random
import seaborn as sns 

import warnings
warnings.filterwarnings('ignore')

# Libraries for preprocessing
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.preprocessing import MinMaxScaler

# Libraries for ML models
from sklearn.naive_bayes import GaussianNB
from sklearn.linear_model import LinearRegression
from sklearn.tree import DecisionTreeClassifier
from sklearn.svm import SVC
from sklearn_oblique_tree.oblique import ObliqueTree
from sklearn.ensemble import GradientBoostingClassifier
from sklearn.ensemble import AdaBoostClassifier
from sklearn.ensemble import RandomForestClassifier

# Libraries for metrics
from sklearn.metrics import (accuracy_score,
                             precision_score,
                             recall_score, 
                             f1_score,
                             r2_score)

def dataPreprocessing():
    # Loading dataset
    heart_data = pd.read_csv('cleveland.csv', header = None)

    heart_data.columns = ['age', 'sex', 'cp', 'trestbps', 'chol',
                'fbs', 'restecg', 'thalach', 'exang', 
                'oldpeak', 'slope', 'ca', 'thal', 'target']

    # Preprocess
    heart_data['target'] = heart_data.target.map({0: 0, 1: 1, 2: 1, 3: 1, 4: 1})
    heart_data['thal'] = heart_data.thal.fillna(heart_data.thal.mean())
    heart_data['ca'] = heart_data.ca.fillna(heart_data.ca.mean())

    X = heart_data.iloc[:, :-1].values
    y = heart_data.iloc[:, -1].values

    # Normalization
    scaler = StandardScaler()
    X = scaler.fit_transform(X)
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=101)
    return X_train, X_test, y_train, y_test

# ML models
def get_results(train_x, train_y, test_x, test_y, classifiers): 
    names = []
    accuracy_list = [] 
    precision_list = []
    recall_list = [] 
    f1_list = []
    tree_depth_list = []
    surrogate_accuracy_list = []
    surrogate_r2_list = []
    random_state = 2
    for cls in classifiers: 
        cls.fit(train_x, train_y)

        # Build an decision tree as surrogate model
        # model_dt = DecisionTreeClassifier(criterion='entropy', max_depth=3)

        # Build an oblique decision tree OC1
        tree = ObliqueTree(splitter="oc1", number_of_restarts=10, max_perturbations=5, random_state=random_state)
        
        # Black-box model fitting
        y_preds_train = cls.predict(train_x)
        y_preds_test = cls.predict(test_x)

        # Get numpy array into C-contiguous
        train_x = np.ascontiguousarray(train_x)
        test_x = np.ascontiguousarray(test_x)
        y_preds_train = np.ascontiguousarray(y_preds_train)
        # OC1 fitting
        tree.fit(train_x, y_preds_train)
        surrogate_y_preds = tree.predict(test_x)
        # DT fitting
        # model_dt.fit(train_x, y_preds_train)
        # surrogate_y_preds = model_dt.predict(test_x)
        
        accuracy = round(cls.score(test_x, test_y), 3)
        precision = round(precision_score(y_preds_test, test_y), 3)
        recall = round(recall_score(y_preds_test, test_y), 3)
        f1 = round(f1_score(y_preds_test, test_y), 3)
        tree_depth = tree.treeDepth()
        # tree_depth = model_dt.get_depth()
        surrogate_accuracy = round(accuracy_score(surrogate_y_preds, y_preds_test), 3)
        surrogate_r2 = round(r2_score(y_preds_test, surrogate_y_preds), 3)
        names.append(cls.__class__.__name__)
        accuracy_list.append(accuracy)
        precision_list.append(precision)
        recall_list.append(recall)
        f1_list.append(f1)
        tree_depth_list.append(tree_depth)
        surrogate_accuracy_list.append(surrogate_accuracy)
        surrogate_r2_list.append(surrogate_r2)
    results = {'Models': names,
               'Surrogate Model': 'CART-OC1',
            #    'Surrogate Model': 'CART',
               'Accuracy': accuracy_list,
               'Precision': precision_list,
               'Recall': recall_list, 
               'F1': f1_list,
               'Tree Depth': tree_depth_list,
               'Surrogate Accuracy': surrogate_accuracy_list,
               'Surrogate R2 Squared': surrogate_r2_list}
    
    resultsDF = pd.DataFrame.from_dict(results)
    
    return resultsDF

if __name__ == "__main__":
    X_train, X_test, y_train, y_test = dataPreprocessing()
    model_svc = SVC(kernel = 'rbf')
    model_rf = RandomForestClassifier(n_estimators=400, min_samples_leaf=0.12, random_state=101)
    classifiers = [model_svc, model_rf]
    results = get_results(X_train, y_train, X_test, y_test, classifiers)
    with pd.option_context('display.max_rows', None, 'display.max_columns', None):  # more options can be specified also
        print(results)