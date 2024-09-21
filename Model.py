import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.tree import DecisionTreeClassifier
from xgboost import XGBClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, classification_report
from sklearn.preprocessing import StandardScaler
import joblib

data = pd.read_csv(r'D:\Projects\LifeBoat\lifeboat-sensor-suite\Dataset\Backup.csv')

data['Risk Category'] = data['Risk Category'].apply(lambda x: 1 if x == 'High Risk' else 0)

data.dropna(inplace=True)

X = data[['Heart Rate']]
y = data['Risk Category']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

models = {
    "Logistic Regression": LogisticRegression(),
    "Decision Tree": DecisionTreeClassifier(),
    "XGBoost": XGBClassifier(use_label_encoder=False, eval_metric='logloss'),
    "Random Forest": RandomForestClassifier(n_estimators=100, random_state=42)
    
}

for model_name, model in models.items():
    print(f"Training {model_name}...")

    model.fit(X_train, y_train)
    
    y_pred = model.predict(X_test)
    
    accuracy = accuracy_score(y_test, y_pred)
    print(f"{model_name} Accuracy: {accuracy * 100:.2f}%")
    
    print(f"{model_name} Classification Report:")
    print(classification_report(y_test, y_pred))
    
    print(f"{model_name} Confusion Matrix:")
    print(confusion_matrix(y_test, y_pred))
    print("\n" + "="*60 + "\n")

joblib.dump(scaler, 'scaler.pkl')
rf_model = models['Random Forest']
joblib.dump(rf_model, 'random_forest_heart_rate_risk_model.pkl')
