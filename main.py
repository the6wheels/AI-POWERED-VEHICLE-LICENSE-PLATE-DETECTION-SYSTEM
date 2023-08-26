import cv2
import easyocr
import pytesseract


# Lecture de l'image
image = cv2.imread("received_image.jpg")

# Redimensionnement de l'image pour une meilleure performance
image = cv2.resize(image,(1000,600), interpolation = cv2.INTER_AREA)

# Conversion de l'image en niveau de gris
grayscale = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

# Filtrage de l'image pour réduire le bruit
blurred = cv2.GaussianBlur(grayscale, (5, 5), 0)

# Détection de contours en utilisant Canny
edged = cv2.Canny(blurred, 30, 150)

# Recherche des contours de l'image
contours, _ = cv2.findContours(edged.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

# Trier les contours par aire en ordre décroissant
contours = sorted(contours, key=cv2.contourArea, reverse=True)[:30]

# Stockage du contour de la plaque d'immatriculation
license_plate_cnt = None

# Parcours de tous les contours pour trouver celui qui correspond à une plaque d'immatriculation
for c in contours:
    # Approximation du contour de la plaque d'immatriculation
    contour_perimeter = cv2.arcLength(c, True)
    approx = cv2.approxPolyDP(c, 0.018 * contour_perimeter, True)
    # Vérification que le contour a 4 coins
    if len(approx) == 4:
        license_plate_cnt = approx
        # Extraction de la plaque d'immatriculation à partir de l'image originale
        [x, y, w, h] = cv2.boundingRect(c)
        license_plate_img = image[y: y + h, x: x + w]
        break

# Si une plaque d'immatriculation est trouvée
if license_plate_cnt is not None:
    # Dessin du contour de la plaque d'immatriculation sur l'image originale
    cv2.drawContours(image, [license_plate_cnt], -1, (0, 255, 0), 3)

    # Extraction du texte de la plaque d'immatriculation à l'aide de PyTesseract
    license_plate_text = pytesseract.image_to_string(license_plate_img, lang='eng')

    # Extraction du texte de la plaque d'immatriculation à l'aide de EasyOCR
    reader = easyocr.Reader(['en'], gpu=False)
    text = reader.readtext(license_plate_img, detail=0, paragraph=False)

    # Affichage du texte extrait

    #print("License plate is:", stat)
    print("Number plate is:", text)


    # Affichage de l'image de la plaque d'immatriculation
    cv2.imshow("cropped license plate", license_plate_img)

# Affichage de l'image originale
cv2.imshow("original image", image)




# Your code logic
def extract_numbers(string):

    numbers = re.findall(r'\d+', string)

    return ''.join(numbers)

print(text)


#cv2.waitKey(0)
#cv2.destroyAllWindows()
