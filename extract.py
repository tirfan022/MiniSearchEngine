import os
root_dir = './data/20_newsgroups'

def load_data(root):
    for category in os.listdir(root):
        category_path = os.path.join(root, category)
        if os.path.isdir(category_path):
            print(f"Processing category: {category}")
            for file_name in os.listdir(category_path):
                file_path = os.path.join(category_path, file_name)
                
                with open(file_path, 'r', encoding='latin-1') as f:
                    content = f.read()
                    # Now you have the raw text! 
                    # Step 1 is to split headers from the body.
                    parts = content.split('\n\n', 1)
                    body = parts[1] if len(parts) > 1 else ""
                    # Proceed to Tokenization...


if __name__ == '__main__':
    if not os.path.isdir(root_dir):
        print(f"Data directory not found: {root_dir}")
    else:
        actual_root = root_dir
        nested = os.path.join(root_dir, '20_newsgroups')
        if os.path.isdir(nested):
            actual_root = nested
        load_data(actual_root)